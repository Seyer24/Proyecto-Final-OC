#ifndef WEBPAGE_H
#define WEBPAGE_H

const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Proyecto Final</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Rajdhani:wght@400;600;700&display=swap');
  :root {
    --bg: #0a0f1e;
    --card: #111827;
    --border: #1e3a5f;
    --accent: #00d4ff;
    --accent2: #ff4d6d;
    --green: #00ff88;
    --text: #e2e8f0;
    --dim: #64748b;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    background: var(--bg);
    color: var(--text);
    font-family: 'Rajdhani', sans-serif;
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 24px 16px;
  }
  .header { text-align: center; margin-bottom: 32px; }
  .header h1 {
    font-family: 'Share Tech Mono', monospace;
    font-size: 28px;
    color: var(--accent);
    letter-spacing: 4px;
    text-transform: uppercase;
    text-shadow: 0 0 20px rgba(0,212,255,0.4);
  }
  .header .autor {
    font-size: 14px;
    color: var(--dim);
    letter-spacing: 2px;
    margin-top: 6px;
    font-family: 'Share Tech Mono', monospace;
  }
  .header .subtitle {
    font-size: 12px;
    color: var(--dim);
    margin-top: 4px;
    font-family: 'Share Tech Mono', monospace;
  }
  .status-bar {
    display: flex;
    align-items: center;
    gap: 8px;
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 8px;
    padding: 8px 16px;
    margin-bottom: 28px;
    font-family: 'Share Tech Mono', monospace;
    font-size: 12px;
    color: var(--dim);
  }
  .dot {
    width: 8px; height: 8px;
    border-radius: 50%;
    background: var(--green);
    box-shadow: 0 0 8px var(--green);
    animation: blink 1.5s infinite;
  }
  @keyframes blink {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.3; }
  }
  .cards {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    gap: 16px;
    width: 100%;
    max-width: 800px;
    margin-bottom: 24px;
  }
  .card {
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 12px;
    padding: 24px 20px;
    text-align: center;
    position: relative;
    overflow: hidden;
  }
  .card::before {
    content: '';
    position: absolute;
    top: 0; left: 0; right: 0;
    height: 2px;
    background: var(--accent);
  }
  .card.green::before { background: var(--green); }
  .card .icon { font-size: 28px; margin-bottom: 8px; }
  .card .label {
    font-size: 11px;
    color: var(--dim);
    letter-spacing: 2px;
    text-transform: uppercase;
    font-family: 'Share Tech Mono', monospace;
    margin-bottom: 8px;
  }
  .card .value {
    font-family: 'Share Tech Mono', monospace;
    font-size: 48px;
    font-weight: 700;
    color: var(--accent);
    line-height: 1;
    transition: color 0.3s;
  }
  .card.green .value { color: var(--green); }
  .card .unit {
    font-size: 14px;
    color: var(--dim);
    margin-top: 4px;
    font-family: 'Share Tech Mono', monospace;
  }
  .stats {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 12px;
    width: 100%;
    max-width: 800px;
    margin-bottom: 24px;
  }
  .stat {
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 10px;
    padding: 16px;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  .stat .slabel {
    font-size: 12px;
    color: var(--dim);
    font-family: 'Share Tech Mono', monospace;
    letter-spacing: 1px;
  }
  .stat .svalue {
    font-family: 'Share Tech Mono', monospace;
    font-size: 22px;
    color: var(--text);
    font-weight: 700;
  }
  .estado-box {
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 10px;
    padding: 14px 20px;
    width: 100%;
    max-width: 800px;
    font-family: 'Share Tech Mono', monospace;
    font-size: 12px;
    color: var(--dim);
    text-align: center;
    margin-bottom: 16px;
  }
  .no-data {
    font-family: 'Share Tech Mono', monospace;
    font-size: 14px;
    color: var(--dim);
    text-align: center;
    padding: 40px;
    border: 1px dashed var(--border);
    border-radius: 12px;
    width: 100%;
    max-width: 800px;
  }

  /* ── Gráfica ─────────────────────────────────────── */
  .chart-box {
    background: var(--card);
    border: 1px solid var(--border);
    border-radius: 12px;
    padding: 16px 20px 12px;
    width: 100%;
    max-width: 800px;
    margin-bottom: 24px;
    position: relative;
  }
  .chart-title {
    font-family: 'Share Tech Mono', monospace;
    font-size: 11px;
    color: var(--dim);
    letter-spacing: 2px;
    text-transform: uppercase;
    margin-bottom: 10px;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  .chart-live {
    font-size: 10px;
    color: var(--green);
    display: flex;
    align-items: center;
    gap: 5px;
  }
  .chart-live::before {
    content: '';
    width: 6px; height: 6px;
    border-radius: 50%;
    background: var(--green);
    box-shadow: 0 0 6px var(--green);
    animation: blink 1s infinite;
  }
  canvas#bpmChart {
    width: 100% !important;
    height: 140px !important;
    display: block;
  }
  .chart-labels {
    display: flex;
    justify-content: space-between;
    font-family: 'Share Tech Mono', monospace;
    font-size: 10px;
    color: var(--dim);
    margin-top: 4px;
  }

  .footer {
    font-family: 'Share Tech Mono', monospace;
    font-size: 10px;
    color: var(--dim);
    margin-top: 16px;
    letter-spacing: 1px;
  }
</style>
</head>
<body>

<div class="header">
  <h1>Proyecto Final</h1>
  <div class="autor">Arath Reyes Gonzalez</div>
  <div class="subtitle">Monitor Cardiaco IoT &mdash; ESP32 + MAX30102</div>
</div>

<div class="status-bar">
  <div class="dot"></div>
  <span id="estado-txt">Cargando...</span>
  &nbsp;|&nbsp; IP: %IP% &nbsp;|&nbsp;
  <span id="timer">Actualizando...</span>
</div>

<div class="estado-box" id="estado-box">Estado: --</div>

<!-- Gráfica siempre visible -->
<div class="chart-box">
  <div class="chart-title">
    <span>&#9656; BPM EN TIEMPO REAL</span>
    <span class="chart-live" id="live-label">EN VIVO</span>
  </div>
  <canvas id="bpmChart"></canvas>
  <div class="chart-labels">
    <span>&#8592; últimos 40 puntos</span>
    <span id="bpm-actual-label">BPM actual: --</span>
  </div>
</div>

<div id="contenido">
  <div class="no-data">
    &#9203; Esperando primera medicion completa...<br><br>
    Coloca el dedo en el sensor y mantenerlo 60 segundos.
  </div>
</div>

<div class="footer">UABC FCITEC 2026-1 &nbsp;|&nbsp; ESP32 + MAX30102</div>

<script>
  // ── Configuración gráfica ──────────────────────────
  const MAX_POINTS = 40;
  const bpmHistory = [];
  const canvas     = document.getElementById('bpmChart');
  const ctx        = canvas.getContext('2d');

  function resizeCanvas() {
    canvas.width  = canvas.offsetWidth;
    canvas.height = 140;
  }
  resizeCanvas();
  window.addEventListener('resize', () => { resizeCanvas(); drawChart(); });

  function drawChart() {
    const W = canvas.width;
    const H = canvas.height;
    ctx.clearRect(0, 0, W, H);

    // Fondo
    ctx.fillStyle = '#111827';
    ctx.fillRect(0, 0, W, H);

    // Sin datos aún
    if (bpmHistory.length < 2) {
      ctx.fillStyle = '#64748b';
      ctx.font = '12px Share Tech Mono, monospace';
      ctx.textAlign = 'center';
      ctx.fillText('Esperando datos de BPM...', W / 2, H / 2);
      return;
    }

    const pad   = { top: 14, bottom: 14, left: 36, right: 10 };
    const chartW = W - pad.left - pad.right;
    const chartH = H - pad.top  - pad.bottom;

    const vals   = bpmHistory.map(p => p.v);
    const minVal = Math.max(30,  Math.min(...vals) - 10);
    const maxVal = Math.min(220, Math.max(...vals) + 10);
    const range  = maxVal - minVal || 1;

    const toX = i  => pad.left + (i / (MAX_POINTS - 1)) * chartW;
    const toY = v  => pad.top  + chartH - ((v - minVal) / range) * chartH;

    // Grid lines
    ctx.strokeStyle = '#1e3a5f';
    ctx.lineWidth   = 0.5;
    for (let i = 0; i <= 4; i++) {
      const y = pad.top + (chartH / 4) * i;
      ctx.beginPath();
      ctx.moveTo(pad.left, y);
      ctx.lineTo(W - pad.right, y);
      ctx.stroke();
      const label = Math.round(maxVal - (range / 4) * i);
      ctx.fillStyle = '#64748b';
      ctx.font      = '9px Share Tech Mono, monospace';
      ctx.textAlign = 'right';
      ctx.fillText(label, pad.left - 4, y + 3);
    }

    // Relleno degradado bajo la curva
    const grad = ctx.createLinearGradient(0, pad.top, 0, pad.top + chartH);
    grad.addColorStop(0,   'rgba(0,212,255,0.25)');
    grad.addColorStop(1,   'rgba(0,212,255,0)');
    ctx.beginPath();
    bpmHistory.forEach((p, i) => {
      const x = toX(i + (MAX_POINTS - bpmHistory.length));
      const y = toY(p.v);
      i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
    });
    const lastX = toX(MAX_POINTS - 1);
    ctx.lineTo(lastX, pad.top + chartH);
    ctx.lineTo(toX(MAX_POINTS - bpmHistory.length), pad.top + chartH);
    ctx.closePath();
    ctx.fillStyle = grad;
    ctx.fill();

    // Línea principal
    ctx.beginPath();
    ctx.strokeStyle = '#00d4ff';
    ctx.lineWidth   = 2;
    ctx.lineJoin    = 'round';
    ctx.lineCap     = 'round';
    bpmHistory.forEach((p, i) => {
      const x = toX(i + (MAX_POINTS - bpmHistory.length));
      const y = toY(p.v);
      i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
    });
    ctx.stroke();

    // Punto final (último valor)
    const lastP = bpmHistory[bpmHistory.length - 1];
    const lx    = toX(MAX_POINTS - 1);
    const ly    = toY(lastP.v);
    ctx.beginPath();
    ctx.arc(lx, ly, 4, 0, Math.PI * 2);
    ctx.fillStyle = '#00d4ff';
    ctx.fill();
    ctx.beginPath();
    ctx.arc(lx, ly, 7, 0, Math.PI * 2);
    ctx.strokeStyle = 'rgba(0,212,255,0.3)';
    ctx.lineWidth   = 2;
    ctx.stroke();

    // Etiqueta del último valor
    ctx.fillStyle = '#00d4ff';
    ctx.font      = 'bold 11px Share Tech Mono, monospace';
    ctx.textAlign = 'left';
    ctx.fillText(lastP.v + ' BPM', lx + 8, ly + 4);
  }

  function pushBpm(v) {
    if (v > 0) {
      bpmHistory.push({ v });
      if (bpmHistory.length > MAX_POINTS) bpmHistory.shift();
    }
    drawChart();
  }

  // ── Estados y helpers ──────────────────────────────
  const ESTADOS = [
    "⏳ ESPERANDO DEDO",
    "❤️ MIDIENDO...",
    "📊 MOSTRANDO RESULTADO",
    "☁️ SUBIENDO A SHEETS"
  ];

  const SPO2_STATUS = v => {
    if (v >= 95) return { txt: "NORMAL",  color: "var(--green)"   };
    if (v >= 90) return { txt: "BAJO",    color: "orange"         };
    if (v >  0)  return { txt: "CRITICO", color: "var(--accent2)" };
    return             { txt: "--",      color: "var(--dim)"      };
  };

  function renderDatos(d) {
    document.getElementById("estado-txt").textContent = ESTADOS[d.estado] || "--";
    document.getElementById("estado-box").textContent = "ESTADO: " + (ESTADOS[d.estado] || "--");

    // Actualiza gráfica con bpm_actual (tiempo real)
    const bpmLive = d.bpm_actual || 0;
    document.getElementById("bpm-actual-label").textContent =
      bpmLive > 0 ? "BPM actual: " + bpmLive : "BPM actual: --";
    pushBpm(bpmLive);

    if (!d.hay_datos) {
      document.getElementById("contenido").innerHTML = `
        <div class="no-data">
          &#9203; Esperando primera medicion completa...<br><br>
          Coloca el dedo en el sensor y mantenerlo 60 segundos.
        </div>`;
      return;
    }

    const spo2 = SPO2_STATUS(d.spo2);

    document.getElementById("contenido").innerHTML = `
      <div class="cards">
        <div class="card">
          <div class="icon">&#10084;&#65039;</div>
          <div class="label">BPM Promedio</div>
          <div class="value" id="val-bpm">${d.bpm}</div>
          <div class="unit">latidos / min</div>
        </div>
        <div class="card green">
          <div class="icon">&#129978;</div>
          <div class="label">SpO2</div>
          <div class="value">${d.spo2 > 0 ? d.spo2 : "--"}</div>
          <div class="unit">% saturacion</div>
        </div>
      </div>
      <div class="stats">
        <div class="stat">
          <div class="slabel">BPM MINIMO</div>
          <div class="svalue">${d.bpm_min}</div>
        </div>
        <div class="stat">
          <div class="slabel">BPM MAXIMO</div>
          <div class="svalue">${d.bpm_max}</div>
        </div>
        <div class="stat">
          <div class="slabel">ULTIMA MEDICION</div>
          <div class="svalue" style="font-size:14px">${d.fecha}</div>
        </div>
        <div class="stat">
          <div class="slabel">ESTADO SpO2</div>
          <div class="svalue" style="color:${spo2.color};font-size:16px">${spo2.txt}</div>
        </div>
      </div>`;
  }

  async function fetchDatos() {
    try {
      const res = await fetch("/datos");
      const d   = await res.json();
      renderDatos(d);
    } catch(e) {
      document.getElementById("estado-txt").textContent = "Sin conexion";
    }
  }

  // ── Refresh cada 1.5 segundos ──────────────────────
  const REFRESH_MS  = 1500;
  let   cuentaMs    = 0;
  const TICK_MS     = 100;

  setInterval(() => {
    cuentaMs += TICK_MS;
    const restante = ((REFRESH_MS - cuentaMs) / 1000).toFixed(1);
    document.getElementById("timer").textContent = "Refresh en " + restante + "s";
    if (cuentaMs >= REFRESH_MS) {
      cuentaMs = 0;
      fetchDatos();
    }
  }, TICK_MS);

  fetchDatos(); // carga inicial
</script>
</body>
</html>
)rawhtml";

#endif
