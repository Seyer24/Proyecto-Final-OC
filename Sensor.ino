#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <spo2_algorithm.h>
#include "webpage.h"

// ── Configuración Access Point ──────────────────────────
const char* AP_SSID     = "X";   // nombre de la red WiFi que crea el ESP32
const char* AP_PASSWORD = "Y";            // contraseña (mínimo 8 caracteres)
// IP fija del ESP32 en modo AP: 192.168.4.1
// No se necesita SCRIPT_URL ni NTP en modo AP (sin internet)

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDR     0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;
WebServer server(80);

// ── Buffers SpO2 ────────────────────────────────────────
#define BUFFER_LENGTH 100
uint32_t irBuffer[BUFFER_LENGTH];
uint32_t redBuffer[BUFFER_LENGTH];
int32_t  spo2Value;
int8_t   spo2Valid;
int32_t  heartRate;
int8_t   hrValid;

// ── Variables BPM ───────────────────────────────────────
// CAMBIO: RATE_SIZE aumentado de 4 a 8 para promedio más estable pero detección más rápida
const byte RATE_SIZE = 4;
byte   rates[RATE_SIZE];
byte   rateSpot  = 0;
long   lastBeat  = 0;
float  beatsPerMinute;
int    beatAvg   = 0;
int    bpmMin    = 999;
int    bpmMax    = 0;

// ── Estado de medición ──────────────────────────────────
enum Estado { ESPERANDO, MIDIENDO, MOSTRANDO_RESULTADO, SUBIENDO };
Estado estadoActual = ESPERANDO;

unsigned long inicioMedicion  = 0;
unsigned long lastDisplay     = 0;
const long TIEMPO_MEDICION    = 60000;
const long DISPLAY_INTERVAL   = 100;

bool   wifiOk    = false;
String ipAddress = "";

// ── Última medición para la web ─────────────────────────
int    ultimoBPM    = 0;
int    ultimoMin    = 0;
int    ultimoMax    = 0;
int    ultimoSpO2   = 0;
String ultimaFecha  = "--";
bool   hayDatos     = false;

// ── BPM en tiempo real para la gráfica ─────────────────
int    bpmActual    = 0;   // se actualiza en cada latido detectado

// ── Prototipos ──────────────────────────────────────────
void pantalla(const char* l1, const char* l2, const char* l3, bool invertir);
void mostrarEsperando();
void mostrarMidiendo(long irValue, int segundos);
void mostrarResultado();
void mostrarExito();
bool subirDatos(int promedio, int spo2);
void calibrarSensor();
void handleRoot();
void handleDatos();

// ── Servidor web ────────────────────────────────────────
void handleRoot() {
  String page = String(INDEX_HTML);
  page.replace("%IP%", ipAddress);
  server.send(200, "text/html; charset=utf-8", page);
}

void handleDatos() {
  StaticJsonDocument<300> doc;
  doc["bpm"]        = ultimoBPM;
  doc["bpm_actual"] = bpmActual;   // BPM en tiempo real para la gráfica
  doc["spo2"]       = ultimoSpO2;
  doc["bpm_min"]    = ultimoMin;
  doc["bpm_max"]    = ultimoMax;
  doc["fecha"]      = ultimaFecha;
  doc["estado"]     = (int)estadoActual;
  doc["hay_datos"]  = hayDatos;
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// ───────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== INICIO ===");

  Wire.begin(21, 22);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERROR: OLED no encontrada");
    while (true);
  }
  Serial.println("OLED OK");
  pantalla("Iniciando...", "", "", false);
  delay(800);

  // ── Modo Access Point ──────────────────────────────
  pantalla("Creando", "red WiFi...", "", false);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  delay(500);

  ipAddress = WiFi.softAPIP().toString();   // siempre 192.168.4.1
  wifiOk    = true;

  Serial.print("AP creado. IP: ");
  Serial.println(ipAddress);

  pantalla("Red creada!", ipAddress.c_str(), AP_SSID, false);

  server.on("/",      handleRoot);
  server.on("/datos", handleDatos);
  server.begin();
  Serial.println("Servidor web: http://" + ipAddress);
  delay(1500);

  // MAX30102
  Serial.println("Iniciando MAX30102...");
  pantalla("Iniciando", "sensor...", "", false);
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("ERROR: MAX30102 no encontrado");
    pantalla("ERROR", "MAX30102", "Revisa cables", false);
    while (true);
  }
  Serial.println("MAX30102 OK");

  // ── CONFIGURACIoN MaS SENSIBLE ──────────────────────
  // setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange)
  // ledBrightness: 60  → más brillo = mejor penetración del dedo
  // sampleAverage: 4   → menos promedio = más sensible a cambios rápidos
  // ledMode:       2   → rojo + IR (necesario para SpO2)
  // sampleRate:    400 → más muestras por segundo = detecta latidos mejor
  // pulseWidth:    411 → máximo ancho de pulso = mejor señal
  // adcRange:      4096→ rango máximo ADC
  particleSensor.setup(60, 4, 2, 400, 411, 4096);
  particleSensor.setPulseAmplitudeRed(0x1F);   // amplitud LED rojo media-alta
  particleSensor.setPulseAmplitudeIR(0x1F);    // amplitud LED IR  media-alta
  Serial.println("Sensor configurado (modo sensible)");

  calibrarSensor();

  Serial.println("=== LISTO ===");
  estadoActual = ESPERANDO;
}

// ── Calibración con dedo ────────────────────────────────
void calibrarSensor() {
  pantalla("Pon el dedo", "en el sensor", "para calibrar", false);
  Serial.println("Esperando dedo para calibrar...");

  particleSensor.check();
  while (particleSensor.getIR() < 30000) {
    particleSensor.check();
    server.handleClient();
    delay(10);
  }

  Serial.println("Dedo detectado, calibrando...");
  pantalla("Calibrando...", "No muevas", "el dedo", false);

  for (byte i = 0; i < BUFFER_LENGTH; i++) {
    while (!particleSensor.available())
      particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i]  = particleSensor.getIR();
    particleSensor.nextSample();
    if (i % 25 == 0) {
      Serial.print("Buffer: ");
      Serial.print(i);
      Serial.println("/100");
    }
  }

  maxim_heart_rate_and_oxygen_saturation(
    irBuffer, BUFFER_LENGTH, redBuffer,
    &spo2Value, &spo2Valid, &heartRate, &hrValid
  );

  Serial.println("Calibracion completa");
  pantalla("Listo!", "Retira el dedo", "para empezar", false);
  delay(2000);
}

// ───────────────────────────────────────────────────────
void loop() {
  server.handleClient();

  while (!particleSensor.available())
    particleSensor.check();

  long irValue = particleSensor.getIR();
  // CAMBIO: umbral inferior bajado de 30000 a 20000 → detecta dedos más fácil
  bool dedoPresente = (irValue > 20000 && irValue < 200000);

  switch (estadoActual) {

    case ESPERANDO:
      mostrarEsperando();
      if (dedoPresente) {
        bpmMin    = 999;
        bpmMax    = 0;
        beatAvg   = 0;
        bpmActual = 0;
        rateSpot  = 0;
        lastBeat  = 0;
        memset(rates, 0, sizeof(rates));
        inicioMedicion = millis();
        estadoActual   = MIDIENDO;
        Serial.println("Medicion iniciada");
      }
      particleSensor.nextSample();
      break;

    case MIDIENDO: {
      if (!dedoPresente) {
        Serial.println("Dedo retirado, reiniciando");
        pantalla("Dedo retirado", "Reiniciando...", "", false);
        bpmActual = 0;
        delay(1500);
        estadoActual = ESPERANDO;
        particleSensor.nextSample();
        break;
      }

      unsigned long tiempoTranscurrido = millis() - inicioMedicion;
      int segundosRestantes = (TIEMPO_MEDICION - tiempoTranscurrido) / 1000;

      if (checkForBeat(irValue)) {
        long delta = millis() - lastBeat;
        lastBeat = millis();

        // CAMBIO: rango ampliado de 300-1500 ms a 250-1800 ms
        // → detecta pulsos más rápidos (hasta 240 BPM) y más lentos (hasta 33 BPM)
        if (delta > 250 && delta < 1800) {
          beatsPerMinute = 60.0 / (delta / 1000.0);

          // CAMBIO: rango de BPM válido ampliado de 40-180 a 30-200
          if (beatsPerMinute > 30 && beatsPerMinute < 200) {
            bpmActual = (int)beatsPerMinute;   // actualiza tiempo real

            rates[rateSpot++] = (byte)beatsPerMinute;
            rateSpot %= RATE_SIZE;
            beatAvg = 0;
            for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
            beatAvg /= RATE_SIZE;

            if ((int)beatsPerMinute > bpmMax) bpmMax = (int)beatsPerMinute;
            if ((int)beatsPerMinute < bpmMin) bpmMin = (int)beatsPerMinute;

            Serial.print("Latido! BPM: ");
            Serial.print(beatsPerMinute);
            Serial.print("  Avg: ");
            Serial.println(beatAvg);
          }
        }
      }

      // Actualizar buffer para SpO2
      for (byte i = 0; i < BUFFER_LENGTH - 1; i++) {
        redBuffer[i] = redBuffer[i + 1];
        irBuffer[i]  = irBuffer[i + 1];
      }
      redBuffer[BUFFER_LENGTH - 1] = particleSensor.getRed();
      irBuffer[BUFFER_LENGTH - 1]  = irValue;

      // Recalcular SpO2 cada 25 muestras
      static byte spo2Counter = 0;
      if (++spo2Counter >= 25) {
        spo2Counter = 0;
        maxim_heart_rate_and_oxygen_saturation(
          irBuffer, BUFFER_LENGTH, redBuffer,
          &spo2Value, &spo2Valid, &heartRate, &hrValid
        );
      }

      if (millis() - lastDisplay > DISPLAY_INTERVAL) {
        lastDisplay = millis();
        mostrarMidiendo(irValue, segundosRestantes);
      }

      if (tiempoTranscurrido >= TIEMPO_MEDICION) {
        Serial.println("Medicion completa");
        Serial.print("BPM promedio: "); Serial.println(beatAvg);
        Serial.print("SpO2: ");        Serial.println(spo2Value);

        ultimoBPM  = beatAvg;
        ultimoMin  = (bpmMin < 999) ? bpmMin : 0;
        ultimoMax  = bpmMax;
        ultimoSpO2 = spo2Valid ? spo2Value : 0;
        hayDatos   = true;

        // Sin NTP en modo AP — usamos tiempo transcurrido desde inicio
        unsigned long seg = millis() / 1000;
        unsigned long min = seg / 60;
        unsigned long hrs = min / 60;
        char buf[20];
        snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", hrs % 24, min % 60, seg % 60);
        ultimaFecha = String(buf);

        estadoActual = MOSTRANDO_RESULTADO;
      }

      particleSensor.nextSample();
      break;
    }

    case MOSTRANDO_RESULTADO:
      mostrarResultado();
      delay(4000);
      estadoActual = SUBIENDO;
      break;

    case SUBIENDO:
      // En modo AP no hay internet — mostrar datos localmente
      pantalla("Datos listos", "Ver en:", ipAddress.c_str(), false);
      delay(2500);
      estadoActual = ESPERANDO;
      break;
  }
}

// ── Pantalla: esperando dedo ────────────────────────────
void mostrarEsperando() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 4);
  display.println("Monitor cardiaco");
  display.drawLine(0, 14, 128, 14, SSD1306_WHITE);
  display.setCursor(8, 22);
  display.println("Coloca el dedo");
  display.setCursor(8, 34);
  display.println("en el sensor y");
  display.setCursor(8, 46);
  display.println("mantenerlo 60 seg");
  display.fillCircle(56, 58, 4, SSD1306_WHITE);
  display.fillCircle(64, 58, 4, SSD1306_WHITE);
  display.fillTriangle(52, 60, 68, 60, 60, 66, SSD1306_WHITE);
  display.display();
}

// ── Pantalla: midiendo ──────────────────────────────────
void mostrarMidiendo(long irValue, int segundos) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Carmen - Midiendo");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  display.setTextSize(3);
  int xPos = (beatAvg >= 100) ? 2 : 14;
  display.setCursor(xPos, 13);
  display.print(beatAvg > 0 ? String(beatAvg) : "---");
  display.setTextSize(1);
  display.setCursor(86, 18);
  display.print("BPM");

  display.setCursor(0, 38);
  display.print("SpO2: ");
  if (spo2Valid && spo2Value > 0) {
    display.print(spo2Value);
    display.print("%");
  } else {
    display.print("---%");
  }

  display.setCursor(0, 50);
  display.print("Tiempo: ");
  display.print(segundos);
  display.print("s");

  int progreso = map(segundos, 60, 0, 0, 118);
  display.drawRect(5, 59, 118, 5, SSD1306_WHITE);
  display.fillRect(5, 59, progreso, 5, SSD1306_WHITE);
  display.display();
}

// ── Pantalla: resultado ─────────────────────────────────
void mostrarResultado() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 2);
  display.println("-- Resultado --");
  display.drawLine(0, 12, 128, 12, SSD1306_WHITE);
  display.setCursor(0, 18);
  display.print("Promedio: ");
  display.setTextSize(2);
  display.setCursor(0, 28);
  display.print(beatAvg);
  display.print(" BPM");
  display.setTextSize(1);
  display.setCursor(0, 48);
  display.print("SpO2: ");
  if (spo2Valid && spo2Value > 0) {
    display.print(spo2Value);
    display.print("%  ");
  } else {
    display.print("---%  ");
  }
  display.print("Min:");
  display.print(bpmMin < 999 ? bpmMin : 0);
  display.display();
}

// ── Pantalla: éxito ─────────────────────────────────────
void mostrarExito() {
  display.clearDisplay();
  display.fillRect(0, 0, 128, 64, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(28, 6);
  display.println("Listo!");
  display.setTextSize(2);
  display.setCursor(44, 18);
  display.println(":)");
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("BPM: ");
  display.print(beatAvg);
  display.print("  SpO2:");
  if (spo2Valid && spo2Value > 0) {
    display.print(spo2Value);
    display.print("%");
  } else {
    display.print("---%");
  }
  display.setCursor(4, 54);
  display.println("Ver en: 192.168.4.1");
  display.display();
}

// En modo Access Point no hay internet, los datos solo se ven en la web local
bool subirDatos(int promedio, int spo2) {
  // Sin internet en modo AP — función deshabilitada
  Serial.println("Modo AP: datos no se suben a Sheets (sin internet)");
  return false;
}

// ── Pantalla genérica ───────────────────────────────────
void pantalla(const char* l1, const char* l2,
              const char* l3, bool invertir) {
  display.clearDisplay();
  if (invertir) {
    display.fillRect(0, 0, 128, 64, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else {
    display.setTextColor(SSD1306_WHITE);
  }
  display.setTextSize(1);
  display.setCursor(0, 10); display.println(l1);
  if (strlen(l2) > 0) { display.setCursor(0, 26); display.println(l2); }
  if (strlen(l3) > 0) { display.setCursor(0, 42); display.println(l3); }
  display.display();
}
