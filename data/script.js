// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket = null;

function initWebSocket() {
    console.log('Opening WebSocket:', gateway);
    websocket = new WebSocket(gateway);
    websocket.onopen = function () { console.log('WS connected'); };
    websocket.onclose = function () {
        console.log('WS closed, retry in 2s');
        setTimeout(initWebSocket, 2000);
    };
    websocket.onmessage = onMessage;
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(data);
        console.log("📤", data);
    } else {
        console.warn("WS not ready");
    }
}

function onMessage(event) {
    try {
        var d = JSON.parse(event.data);
        if (d.page === 'telemetry' || d.page === 'sensor') {
            if (gaugeTemp && d.temp !== undefined && !isNaN(d.temp)) gaugeTemp.refresh(d.temp);
            if (gaugeHumi && d.hum !== undefined && !isNaN(d.hum)) gaugeHumi.refresh(d.hum);
            if (d.ml !== undefined && !isNaN(d.ml)) updateMlScore(parseFloat(d.ml));
            updateDeviceState(d);
        }
    } catch (e) {
        console.warn("Bad JSON:", event.data);
    }
}

// ==================== ML INFERENCE SCORE ====================
function updateMlScore(score) {
    var scoreEl = document.getElementById('ml_score_text');
    var barEl   = document.getElementById('ml_bar');
    var badgeEl = document.getElementById('ml_badge');
    if (!scoreEl || !barEl || !badgeEl) return;

    var pct = Math.max(0, Math.min(1, score));
    scoreEl.textContent = (pct * 100).toFixed(1) + '%';

    // Progress bar width + color position
    barEl.style.width = (pct * 100) + '%';
    // Shift gradient: 0%=green left, 100%=red right
    barEl.style.backgroundPosition = (pct * 100) + '% 50%';

    // Badge state
    badgeEl.className = 'ml-badge';
    if (pct >= 0.8) {
        badgeEl.textContent = '⚠️ CẢNH BÁO - BẬT QUẠT';
        badgeEl.classList.add('alert');
        scoreEl.style.color = '#b71c1c';
    } else if (pct >= 0.5) {
        badgeEl.textContent = '⚡ CẢNH GIÁC';
        badgeEl.classList.add('warning');
        scoreEl.style.color = '#e65100';
    } else {
        badgeEl.textContent = '✅ BÌNH THƯỜNG';
        badgeEl.classList.add('normal');
        scoreEl.style.color = '#2e7d32';
    }
}

// ==================== UI NAVIGATION ====================
let relayList = [];
let deleteTarget = null;

function showSection(id, event) {
    document.querySelectorAll('.section').forEach(sec => sec.style.display = 'none');
    var target = document.getElementById(id);
    if (target) target.style.display = id === 'settings' ? 'flex' : 'block';
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    if (event && event.currentTarget) event.currentTarget.classList.add('active');
}

// ==================== HOME GAUGES (pure SVG) ====================
class SvgGauge {
    constructor(id, opts) {
        this.id = id;
        this.min = (opts.min !== undefined) ? opts.min : 0;
        this.max = (opts.max !== undefined) ? opts.max : 100;
        this.value = (opts.value !== undefined) ? opts.value : 0;
        this.colors = opts.levelColors || ['#2294F2'];
        this.w = opts.width || 260;
        this.h = opts.height || 260;
        this._render();
    }
    _color(v) {
        const pct = Math.max(0, Math.min(1, (v - this.min) / (this.max - this.min)));
        const i = Math.min(Math.floor(pct * this.colors.length), this.colors.length - 1);
        return this.colors[i];
    }
    _arc(cx, cy, r, a1, a2) {
        const rad = d => d * Math.PI / 180;
        const x1 = cx + r * Math.cos(rad(a1)), y1 = cy + r * Math.sin(rad(a1));
        const x2 = cx + r * Math.cos(rad(a2)), y2 = cy + r * Math.sin(rad(a2));
        const large = (a2 - a1) > 180 ? 1 : 0;
        return `M${x1} ${y1} A${r} ${r} 0 ${large} 1 ${x2} ${y2}`;
    }
    _render() {
        const el = document.getElementById(this.id);
        if (!el) { console.warn("Gauge target not found:", this.id); return; }
        const cx = this.w / 2, cy = this.h / 2 + 10, r = Math.min(this.w, this.h) / 2 - 28;
        const S = 135, SWEEP = 270;
        const pct = Math.max(0, Math.min(1, (this.value - this.min) / (this.max - this.min)));
        const vSweep = pct * SWEEP;
        const color = this._color(this.value);
        const valStr = isNaN(this.value) ? '--' : this.value.toFixed(1);
        const valArc = vSweep > 0.5
            ? `<path d="${this._arc(cx, cy, r, S, S + vSweep)}" fill="none" stroke="${color}" stroke-width="22" stroke-linecap="round"/>`
            : '';
        el.innerHTML =
            `<svg width="${this.w}" height="${this.h}" viewBox="0 0 ${this.w} ${this.h}" xmlns="http://www.w3.org/2000/svg">` +
            `<path d="${this._arc(cx, cy, r, S, S + SWEEP)}" fill="none" stroke="#e0e0e0" stroke-width="22" stroke-linecap="round"/>` +
            valArc +
            `<text x="${cx}" y="${cy + 8}" text-anchor="middle" font-size="46" font-weight="700" fill="#333" font-family="Poppins,Arial,sans-serif">${valStr}</text>` +
            `<text x="${cx}" y="${cy + 32}" text-anchor="middle" font-size="13" fill="#aaa" font-family="Poppins,Arial,sans-serif">${this.min} ~ ${this.max}</text>` +
            `</svg>`;
    }
    refresh(v) {
        const num = parseFloat(v);
        this.value = isNaN(num) ? 0 : parseFloat(num.toFixed(1));
        this._render();
    }
}

var gaugeTemp = null, gaugeHumi = null;

// ==================== FIXED DEVICE CONTROLS ====================
function toggleFixed(name) {
    Send_Data(JSON.stringify({ page: 'control', device: name }));
}

function updateDeviceState(d) {
    setBtn('btnNeo', d.neo, 'ON');
    setBtn('btnBlinky', d.blinky, 'ON');
    setBtn('btnFanAuto', d.fanAuto, 'AUTO');
    setBtn('btnFanState', d.fanState, 'ON', d.fanAuto === 'AUTO');
}

function setBtn(id, val, onWord, disabled) {
    const el = document.getElementById(id);
    if (!el) return;
    el.textContent = val || '--';
    el.classList.toggle('on', val === onWord);
    el.classList.toggle('disabled', !!disabled);
}

// ==================== DYNAMIC RELAY (giữ nguyên) ====================
function openAddRelayDialog() {
    var d = document.getElementById('addRelayDialog');
    if (d) d.style.display = 'flex';
}
function closeAddRelayDialog() {
    var d = document.getElementById('addRelayDialog');
    if (d) d.style.display = 'none';
}
function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();
    if (!name || !gpio) return alert("⚠️ Please fill all fields!");
    relayList.push({ id: Date.now(), name, gpio, state: false });
    renderRelays();
    closeAddRelayDialog();
}
function renderRelays() {
    const container = document.getElementById('relayContainer');
    if (!container) return;
    container.innerHTML = "";
    relayList.forEach(r => {
        const card = document.createElement('div');
        card.className = 'device-card';
        card.innerHTML =
            `<div class="device-icon">⚡</div>` +
            `<h3>${r.name}</h3>` +
            `<p>GPIO: ${r.gpio}</p>` +
            `<button class="toggle-btn ${r.state ? 'on' : ''}" onclick="toggleRelay(${r.id})">${r.state ? 'ON' : 'OFF'}</button>` +
            `<div class="delete-icon" onclick="showDeleteDialog(${r.id})">🗑️</div>`;
        container.appendChild(card);
    });
}
function toggleRelay(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay) {
        relay.state = !relay.state;
        Send_Data(JSON.stringify({
            page: "device",
            value: {
                name: relay.name,
                status: relay.state ? "ON" : "OFF",
                gpio: parseInt(relay.gpio, 10)
            }
        }));
        renderRelays();
    }
}
function showDeleteDialog(id) {
    deleteTarget = id;
    var d = document.getElementById('confirmDeleteDialog');
    if (d) d.style.display = 'flex';
}
function closeConfirmDelete() {
    var d = document.getElementById('confirmDeleteDialog');
    if (d) d.style.display = 'none';
}
function confirmDelete() {
    relayList = relayList.filter(r => r.id !== deleteTarget);
    renderRelays();
    closeConfirmDelete();
}

// ==================== INIT (chạy 1 lần khi DOM sẵn sàng) ====================
function appInit() {
    try {
        gaugeTemp = new SvgGauge('gauge_temp', {
            value: 0, min: -10, max: 50, width: 260, height: 260,
            levelColors: ['#00BCD4', '#4CAF50', '#FFC107', '#F44336']
        });
        gaugeHumi = new SvgGauge('gauge_humi', {
            value: 0, min: 0, max: 100, width: 260, height: 260,
            levelColors: ['#42A5F5', '#00BCD4', '#0288D1']
        });
    } catch (e) {
        console.error("Gauge init failed:", e);
    }

    var form = document.getElementById("settingsForm");
    if (form) {
        form.addEventListener("submit", function (e) {
            e.preventDefault();
            const ssid = document.getElementById("ssid").value.trim();
            const password = document.getElementById("password").value.trim();
            const token = document.getElementById("token").value.trim();
            const server = document.getElementById("server").value.trim();
            const port = document.getElementById("port").value.trim();
            Send_Data(JSON.stringify({
                page: "setting",
                value: { ssid, password, token, server, port }
            }));
            alert("✅ Cấu hình đã được gửi đến thiết bị!");
        });
    }

    // Fetch initial state via REST (shows data before first WS message arrives)
    fetch('/status')
        .then(r => r.json())
        .then(d => {
            if (gaugeTemp && d.temp !== undefined && !isNaN(d.temp)) gaugeTemp.refresh(d.temp);
            if (gaugeHumi && d.hum !== undefined && !isNaN(d.hum)) gaugeHumi.refresh(d.hum);
            if (d.ml !== undefined && !isNaN(d.ml)) updateMlScore(parseFloat(d.ml));
            updateDeviceState(d);
        })
        .catch(() => { /* ESP32 not yet ready, WS will populate later */ });


    initWebSocket();
}

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', appInit);
} else {
    appInit();
}
