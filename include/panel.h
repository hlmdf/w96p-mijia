#pragma once
#include <Arduino.h>
static const char PANEL_HTML[] PROGMEM = R"W96PAGE(<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>W96P 风扇控制 V3.7</title>
<style>
:root {
    --primary-color: #667eea;
    --primary-dark: #764ba2;
    --success-color: #00b894;
    --danger-color: #ff6b6b;
    --warning-color: #fdcb6e;
    --secondary-color: #6c757d;
    --gray-light: #adb5bd;
    --bg-gradient: linear-gradient(135deg, var(--primary-color) 0%, var(--primary-dark) 100%);
    --card-shadow: 0 8px 25px rgba(0,0,0,0.1);
    --border-radius: 16px;
    --border-radius-sm: 12px;
}

* { box-sizing: border-box; -webkit-tap-highlight-color: transparent; margin: 0; padding: 0; }

body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    background: var(--bg-gradient);
    min-height: 100vh;
    color: #333;
    padding: 10px;
    overflow-x: hidden;
}

.container { max-width: 500px; margin: 0 auto; padding-bottom: 20px; }

.header {
    text-align: center; color: white; margin-bottom: 15px; padding: 15px;
    background: rgba(255,255,255,0.1); border-radius: var(--border-radius); backdrop-filter: blur(10px);
}
.header h1 { margin: 0; font-size: 22px; font-weight: 600; }
.header p { margin: 5px 0 0 0; opacity: 0.9; font-size: 13px; }

.card {
    background: white; border-radius: var(--border-radius); padding: 15px;
    margin-bottom: 12px; box-shadow: var(--card-shadow); overflow: hidden;
}
.card h3 { margin: 0 0 12px 0; color: #333; font-size: 16px; display: flex; align-items: center; }
.card h3::before {
    content: ""; width: 4px; height: 16px; background: var(--primary-color);
    border-radius: 2px; margin-right: 8px; flex-shrink: 0;
}

.device-info-refresh-btn { margin-left: auto; padding: 6px 12px; font-size: 12px; min-height: 30px; border-radius: 8px; flex-shrink: 0; }

.btn {
    border: none; border-radius: var(--border-radius-sm); padding: 12px 16px; font-size: 14px;
    font-weight: 600; cursor: pointer; transition: all 0.2s ease; display: inline-flex;
    align-items: center; justify-content: center; min-height: 44px; touch-action: manipulation;
    white-space: nowrap; user-select: none; background: var(--bg-gradient); color: white;
    box-shadow: 0 4px 15px rgba(102, 126, 234, 0.3);
}
.btn:active { transform: scale(0.96); opacity: 0.9; box-shadow: 0 2px 8px rgba(102, 126, 234, 0.3); }
.btn-danger { background: linear-gradient(135deg, var(--danger-color) 0%, #ee5a24 100%); box-shadow: 0 4px 15px rgba(255, 107, 107, 0.3); }
.btn-success { background: linear-gradient(135deg, var(--success-color) 0%, #00a085 100%); box-shadow: 0 4px 15px rgba(0, 184, 148, 0.3); }
.btn-warning { background: linear-gradient(135deg, var(--warning-color) 0%, #e17055 100%); box-shadow: 0 4px 15px rgba(253, 203, 110, 0.3); }
.btn-secondary { background: linear-gradient(135deg, var(--secondary-color) 0%, #5a6268 100%); box-shadow: 0 4px 15px rgba(108, 117, 125, 0.3); }
.btn-off { background: linear-gradient(135deg, var(--gray-light) 0%, #868e96 100%); box-shadow: 0 4px 15px rgba(173, 181, 189, 0.3); }
.btn-on { background: linear-gradient(135deg, var(--success-color) 0%, #00a085 100%); box-shadow: 0 4px 15px rgba(0, 184, 148, 0.3); }
.btn-turbo-on {
    background: linear-gradient(135deg, #ff4757 0%, #ff6348 100%);
    box-shadow: 0 4px 15px rgba(255, 71, 87, 0.4); animation: turboPulse 2s infinite;
}
@keyframes turboPulse {
    0%, 100% { box-shadow: 0 4px 15px rgba(255, 71, 87, 0.4); }
    50% { box-shadow: 0 4px 25px rgba(255, 71, 87, 0.7); }
}

.connect-btn { width: 100%; margin-bottom: 8px; font-size: 16px; padding: 14px; border-radius: 12px; }

.button-row {
    display: flex; gap: 6px; margin-bottom: 10px; overflow-x: auto; padding: 4px 0;
    scrollbar-width: none; -webkit-overflow-scrolling: touch; flex-wrap: wrap;
}
.button-row::-webkit-scrollbar { display: none; }
.button-row button { flex-shrink: 0; min-width: 56px; padding: 12px 10px; font-size: 13px; font-weight: 700; letter-spacing: 0.5px; }

.light-control-row button { min-width: 70px; }
.speed-controls .button-row button { flex-shrink: 0; min-width: 56px; padding: 12px 10px; font-size: 13px; font-weight: 700; letter-spacing: 0.5px; }
.speed-controls .button-row .nw-btn { min-width: 80px; }
.speed-controls .button-row .turbo-btn { min-width: 100px; }
.gear-down-mode-btn { min-width: 120px !important; font-size: 12px !important; }
.natural-wind .button-row button { flex-shrink: 0; min-width: 65px; padding: 10px 12px; font-size: 14px; }

.input-group { display: flex; align-items: center; margin: 10px 0; gap: 8px; flex-wrap: nowrap; }
.input-group label { min-width: 80px; font-size: 14px; color: #555; flex-shrink: 0; }
.input-wrapper { display: flex; align-items: center; flex: 1; min-width: 0; }
.input-wrapper input {
    flex: 1; min-width: 0; padding: 12px; border: 2px solid #e9ecef;
    border-radius: 10px 0 0 10px; font-size: 15px; background: #f8f9fa; transition: border-color 0.2s;
}
.input-wrapper input:focus { outline: none; border-color: var(--primary-color); background: white; }
.percent-sign {
    padding: 12px; background: #e9ecef; border: 2px solid #e9ecef; border-left: none;
    border-radius: 0 10px 10px 0; font-size: 15px; font-weight: 600; color: #555; flex-shrink: 0;
}

.status { text-align: center; padding: 12px; border-radius: 10px; margin-top: 12px; font-weight: 600; font-size: 15px; }
.status-connected { background: linear-gradient(135deg, #d4edda 0%, #c3e6cb 100%); color: #155724; border: 1px solid #c3e6cb; }
.status-disconnected { background: linear-gradient(135deg, #f8d7da 0%, #f5c6cb 100%); color: #721c24; border: 1px solid #f5c6cb; }

.info-display {
    background: #f8f9fa; border-radius: 8px; padding: 12px; margin-top: 12px;
    text-align: center; font-size: 14px; color: #555; border-left: 4px solid var(--primary-color);
    word-break: break-word;
}

.sleep-delay-input-group, .turbo-time-input-group {
    display: flex; align-items: center; margin: 10px 0; gap: 8px; flex-wrap: nowrap;
}
.sleep-delay-input-group label, .turbo-time-input-group label { min-width: 100px; font-size: 14px; color: #555; flex-shrink: 0; }
.sleep-delay-wrapper, .turbo-time-wrapper { display: flex; align-items: center; flex: 1; }
.sleep-delay-wrapper input, .turbo-time-wrapper input {
    flex: 1; min-width: 0; padding: 12px; border: 2px solid #e9ecef;
    border-radius: 10px 0 0 10px; font-size: 15px; background: #f8f9fa; transition: border-color 0.2s;
}
.sleep-delay-wrapper input:focus, .turbo-time-wrapper input:focus { outline: none; border-color: var(--primary-color); background: white; }
.sleep-delay-unit, .turbo-time-unit {
    padding: 12px; background: #e9ecef; border: 2px solid #e9ecef; border-left: none;
    border-radius: 0 10px 10px 0; font-size: 15px; font-weight: 600; color: #555; flex-shrink: 0;
}
.sleep-delay-buttons, .turbo-time-buttons { display: flex; gap: 8px; margin-top: 12px; }
.sleep-delay-buttons button, .turbo-time-buttons button { flex: 1; }

.turbo-time-presets { display: flex; gap: 6px; margin-top: 8px; flex-wrap: wrap; }
.turbo-time-presets button { flex: 1; min-width: 60px; padding: 8px 6px; font-size: 12px; }

.speed-slider-container { margin: 15px 0; }
.speed-slider-label { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
.speed-slider-label span { font-size: 15px; font-weight: 600; color: #333; }
.speed-value { font-size: 17px; font-weight: 700; color: var(--primary-color); }
.speed-slider {
    width: 100%; height: 10px; border-radius: 5px; background: #e9ecef;
    -webkit-appearance: none; appearance: none; margin: 10px 0; cursor: pointer; touch-action: none;
}
.speed-slider::-webkit-slider-thumb {
    -webkit-appearance: none; width: 30px; height: 30px; border-radius: 50%;
    background: var(--primary-color); cursor: pointer; box-shadow: 0 2px 10px rgba(102, 126, 234, 0.6);
    border: 3px solid white; transition: transform 0.05s ease;
}
.speed-slider::-moz-range-thumb {
    width: 30px; height: 30px; border-radius: 50%; background: var(--primary-color);
    cursor: pointer; border: 3px solid white; box-shadow: 0 2px 10px rgba(102, 126, 234, 0.6);
}
.speed-input-group { display: flex; align-items: center; margin: 12px 0; gap: 8px; }
.speed-input-group label { min-width: 70px; font-size: 14px; color: #555; }
.speed-input-wrapper { display: flex; align-items: center; flex: 1; }
.speed-input-wrapper input {
    flex: 1; padding: 12px; border: 2px solid #e9ecef; border-radius: 10px 0 0 10px;
    font-size: 15px; background: #f8f9fa;
}
.speed-unit {
    padding: 12px; background: #e9ecef; border: 2px solid #e9ecef; border-left: none;
    border-radius: 0 10px 10px 0; font-size: 15px; font-weight: 600; color: #555;
}
.speed-adjust-buttons { display: flex; justify-content: center; gap: 15px; margin: 15px 0; }
.speed-adjust-btn {
    width: 60px; height: 60px; border-radius: 50%; font-size: 28px; font-weight: bold;
    padding: 0; display: flex; align-items: center; justify-content: center;
    user-select: none; -webkit-user-select: none; touch-action: manipulation;
}

.curve-editor { margin-top: 12px; }
.curve-info {
    background: #f8f9fa; border-radius: 8px; padding: 10px; margin: 8px 0;
    font-size: 12px; color: #666; border-left: 4px solid var(--success-color);
}
.curve-textarea {
    width: 100%; height: 70px; padding: 10px; border: 2px solid #e9ecef;
    border-radius: 8px; font-family: "SF Mono", Monaco, monospace; font-size: 11px;
    resize: vertical; background: #f8f9fa; margin-bottom: 8px;
}
.curve-textarea:focus { outline: none; border-color: var(--primary-color); background: white; }
.curve-chart-container {
    background: #f8f9fa; border-radius: 8px; padding: 12px; margin: 12px 0;
    border: 1px solid #e9ecef; position: relative; overflow: hidden; width: 100%; box-sizing: border-box;
}
.curve-chart-title { font-size: 13px; font-weight: 600; color: #333; margin-bottom: 8px; text-align: center; }
.curve-canvas {
    display: block; width: 100%; height: 120px; max-width: 100%; background: white;
    border-radius: 6px; border: 1px solid #ddd; cursor: grab; touch-action: none; user-select: none; box-sizing: border-box;
}
.curve-canvas.dragging { cursor: grabbing; }
.curve-stats { display: flex; justify-content: space-between; margin-top: 8px; font-size: 11px; color: #666; }
.curve-buttons { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; margin-top: 8px; }
.curve-buttons button { width: 100%; font-size: 13px; padding: 10px 6px; white-space: nowrap; }

.edit-hint {
    position: absolute; top: 8px; right: 8px; background: rgba(102, 126, 234, 0.9);
    color: white; padding: 4px 8px; border-radius: 4px; font-size: 11px; pointer-events: none; z-index: 10;
}

.timer-with-presets { display: flex; align-items: center; gap: 8px; margin-bottom: 8px; }
.timer-input-group { flex: 1; }
.preset-buttons { display: flex; gap: 6px; }
.preset-buttons button { min-width: 55px; padding: 8px 10px; font-size: 13px; }

.checkbox-group { display: flex; gap: 12px; margin-top: 8px; flex-wrap: wrap; }
.checkbox-item { display: flex; align-items: center; gap: 4px; }
.checkbox-item input[type="checkbox"] { width: 16px; height: 16px; accent-color: var(--primary-color); }
.checkbox-item label { font-size: 13px; color: #555; cursor: pointer; }

.real-time-status {
    display: grid; grid-template-columns: repeat(2, 1fr); gap: 8px;
    background: #e9f7fe; border-radius: 12px; padding: 12px; margin: 15px 0; border: 1px solid #bee5eb;
}
.status-item { text-align: center; padding: 8px 4px; background: white; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.05); }
.status-label { font-size: 11px; color: #6c757d; margin-bottom: 2px; font-weight: 500; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.status-value { font-size: 14px; font-weight: 700; color: #007bff; line-height: 1.2; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }

.device-info-grid {
    display: grid; grid-template-columns: repeat(2, 1fr); gap: 8px;
    background: #e9f7fe; border-radius: 12px; padding: 12px; margin: 15px 0; border: 1px solid #bee5eb;
}

#toast {
    position: fixed; bottom: 20px; left: 50%; transform: translateX(-50%);
    background: rgba(0,0,0,0.8); color: white; padding: 8px 16px; border-radius: 24px;
    font-size: 13px; z-index: 1000; pointer-events: none; opacity: 0; transition: opacity 0.2s; white-space: nowrap;
}
#toast.show { opacity: 1; }

@media (max-width: 480px) {
    body { padding: 8px; } .container { padding: 8px; }
    .card { padding: 12px; margin-bottom: 10px; border-radius: 12px; }
    .button-row { gap: 4px; } .button-row button { min-width: 50px; padding: 10px 8px; font-size: 12px; }
    .light-control-row button { min-width: 60px; padding: 10px 6px; font-size: 12px; }
    .speed-controls .button-row button { min-width: 50px; padding: 10px 8px; font-size: 12px; }
    .natural-wind .button-row button { min-width: 60px; padding: 8px 10px; font-size: 13px; }
    .input-group { gap: 6px; } .input-group label { min-width: 65px; font-size: 13px; }
    .input-wrapper input { padding: 10px; font-size: 14px; } .percent-sign { padding: 10px; font-size: 14px; }
    .header h1 { font-size: 20px; }
    .speed-input-group { flex-direction: column; align-items: stretch; gap: 6px; }
    .speed-input-group label { min-width: auto; }
    .curve-buttons { grid-template-columns: repeat(3, 1fr); gap: 4px; }
    .curve-buttons button { font-size: 11px; padding: 8px 2px; white-space: normal; word-break: keep-all; line-height: 1.2; }
    .timer-with-presets { flex-direction: column; align-items: stretch; gap: 8px; }
    .preset-buttons { display: grid; grid-template-columns: repeat(2, 1fr); gap: 6px; }
    .preset-buttons button { min-width: auto; width: 100%; }
    .real-time-status { grid-template-columns: repeat(2, 1fr); gap: 6px; padding: 10px; }
    .status-item { padding: 6px 2px; } .status-label { font-size: 10px; } .status-value { font-size: 12px; }
    .device-info-grid { grid-template-columns: repeat(2, 1fr); gap: 6px; padding: 10px; }
    .device-info-refresh-btn { padding: 5px 10px; font-size: 11px; min-height: 28px; }
    .turbo-btn { min-width: 90px !important; font-size: 12px !important; }
    .turbo-time-presets button { min-width: 50px; font-size: 11px; }
    .curve-chart-container { padding: 8px; overflow: hidden; } .curve-canvas { height: 100px; width: 100%; }
    .btn { padding: 10px 12px; font-size: 13px; }
    .connect-btn { padding: 12px; font-size: 15px; }
    .gear-down-mode-btn { min-width: 100px !important; font-size: 11px !important; }
    .speed-adjust-buttons { gap: 10px; margin: 12px 0; }
    .speed-adjust-btn { width: 50px; height: 50px; font-size: 24px; }
    .sleep-delay-input-group, .turbo-time-input-group { flex-direction: column; align-items: stretch; gap: 6px; }
    .sleep-delay-input-group label, .turbo-time-input-group label { min-width: auto; }
    .sleep-delay-buttons, .turbo-time-buttons { flex-direction: column; gap: 6px; }
}
</style>
</head>
<body>
<div class="container"><div class="card"><h3>C3 桥接</h3>
<p id="bridgeStatus" role="status">正在读取连接状态…</p>
<details><summary>配置巴法云</summary><p>创建 w96speed002（风速）和 w96light002（实体灯）主题后填写 UID。云端未配置也能使用本地控制。</p>
<form onsubmit="saveCloud(event)"><label for="bemfaUid">巴法私钥 UID</label><input id="bemfaUid" type="password" autocomplete="off" maxlength="32" required><button class="btn btn-primary" type="submit">保存</button><p id="cloudMessage" role="status"></p></form>
<p>UID 仅在本地填写；此页面通过局域网 HTTP 访问，请使用自己的可信网络。</p></details>
<details><summary>设置 Wi-Fi</summary><form onsubmit="saveWifi(event)"><div class="input-group"><label for="wifiSsid">2.4GHz 名称</label><input id="wifiSsid" maxlength="32" required autocomplete="off"></div><div class="input-group"><label for="wifiPass">Wi-Fi 密码</label><input id="wifiPass" type="password" maxlength="63" autocomplete="new-password"></div><button class="btn btn-primary" type="submit">连接 Wi-Fi</button><p id="wifiMessage" role="status"></p></form></details>
<p>C3 会自动连接风扇；打开面板后点「连接风扇」加载控制状态。开关机或断线后，读数以重新读取结果为准。</p></div></div>
<div class="container">
    <div class="header">
        <h1>🌬️ W96P 风扇控制 V3.7</h1>
        <p>C3 本地控制 · 米家桥接</p>
    </div>
    
    <div class="card">
        <h3>🔗 连接设备</h3>
        <button class="btn btn-primary connect-btn" id="connectBtn">📶 连接风扇</button>
        <button class="btn btn-warning connect-btn" id="disconnectBtn">🔌 断开连接</button>
        <div id="status" class="status status-disconnected">❌ 未连接</div>
    </div>
    
    <div class="card">
        <h3>📱 设备信息
            <button class="btn btn-secondary device-info-refresh-btn" id="refreshDeviceInfoBtn">🔄 获取信息</button>
        </h3>
        <div class="device-info-grid">
            <div class="status-item"><div class="status-label">序列号</div><div class="status-value" id="serialNumberValue">--</div></div>
            <div class="status-item"><div class="status-label">固件版本</div><div class="status-value" id="firmwareVersionValue">--</div></div>
        </div>
    </div>
    
    <div class="card">
        <h3>📊 实时状态</h3>
        <div class="real-time-status">
            <div class="status-item"><div class="status-label">转速</div><div class="status-value" id="fanSpeedValue">-- %</div></div>
            <div class="status-item"><div class="status-label">电池功率</div><div class="status-value" id="powerValue">-- W</div></div>
            <div class="status-item"><div class="status-label">VBUS电流</div><div class="status-value" id="vbusCurrentValue">-- mA</div></div>
            <div class="status-item"><div class="status-label">电池电流</div><div class="status-value" id="batteryCurrentValue">-- mA</div></div>
            <div class="status-item"><div class="status-label">VBUS电压</div><div class="status-value" id="vbusVoltageValue">-- V</div></div>
            <div class="status-item"><div class="status-label">电池电压</div><div class="status-value" id="batteryVoltageValue">-- V</div></div>
            <div class="status-item"><div class="status-label">充放电状态</div><div class="status-value" id="chargeStatusValue">--</div></div>
            <div class="status-item"><div class="status-label">电机电流</div><div class="status-value" id="motorCurrentValue">-- mA</div></div>
            <div class="status-item"><div class="status-label">电机功率</div><div class="status-value" id="motorPowerValue">-- W</div></div>
            <div class="status-item"><div class="status-label">电机电压</div><div class="status-value" id="motorVoltageValue">-- V</div></div>
        </div>
    </div>
    
    <div class="card speed-controls">
        <h3>💨 档位控制</h3>
        <div class="button-row">
            <button class="btn btn-danger" data-power="00">关机</button>
            <button class="btn btn-primary" data-power="01">1档</button>
            <button class="btn btn-primary" data-power="02">2档</button>
            <button class="btn btn-primary" data-power="03">3档</button>
            <button class="btn btn-primary" data-power="04">4档</button>
            <button id="turboBtn" class="btn btn-off turbo-btn">Turbo OFF</button>
            <button id="nwBtn" class="btn btn-off nw-btn">自然风 OFF</button>
            <button id="gearDownModeBtn" class="btn btn-secondary gear-down-mode-btn">减档: 逐级减档</button>
        </div>
    </div>
    
    <div class="card speed-control-card">
        <h3>🌀 转速控制</h3>
        <div class="speed-slider-container">
            <div class="speed-slider-label"><span>当前转速:</span><div class="speed-value" id="currentSpeedValue">50%</div></div>
            <input type="range" min="0" max="100" value="50" step="1" class="speed-slider" id="speedSlider">
            <div class="speed-slider-label"><span>0%</span><span>100%</span></div>
        </div>
        <div class="speed-adjust-buttons">
            <button class="btn btn-secondary speed-adjust-btn" id="decreaseSpeedBtn">−</button>
            <button class="btn btn-secondary speed-adjust-btn" id="increaseSpeedBtn">+</button>
        </div>
        <div class="speed-input-group">
            <label>输入转速:</label>
            <div class="speed-input-wrapper">
                <input type="number" id="manualSpeedInput" min="0" max="100" value="50" placeholder="0-100">
                <span class="speed-unit">%</span>
            </div>
        </div>
    </div>
    
    <div class="card">
        <h3>💡 灯光控制</h3>
        <div class="button-row light-control-row">
            <button class="btn btn-danger light-btn" data-brightness="00">关灯</button>
            <button class="btn btn-primary light-btn" data-brightness="01">低亮度</button>
            <button class="btn btn-primary light-btn" data-brightness="02">中低亮度</button>
            <button class="btn btn-primary light-btn" data-brightness="03">中高亮度</button>
            <button class="btn btn-primary light-btn" data-brightness="04">最高亮度</button>
        </div>
        <div class="info-display" id="lightStatusDisplay">当前: 未设置</div>
    </div>
    
    <div class="card">
        <h3>⏱️ 自定义定时</h3>
        <div class="timer-with-presets">
            <div class="timer-input-group">
                <div class="input-group">
                    <label>定时（分钟）:</label>
                    <input type="number" id="customMinutes" min="1" max="480" value="30">
                </div>
            </div>
            <div class="preset-buttons">
                <button class="btn btn-primary" id="preset1h">1小时</button>
                <button class="btn btn-primary" id="preset4h">4小时</button>
            </div>
        </div>
        <div class="button-row">
            <button class="btn btn-success" id="setTimerBtn">设置定时</button>
            <button class="btn btn-danger" id="cancelTimerBtn">取消定时</button>
            <button class="btn btn-secondary" id="readTimerBtn">读取剩余</button>
        </div>
        <div class="info-display" id="remainingTimeDisplay">剩余时间: 未设置</div>
    </div>
    
    <div class="card">
        <h3>🚀 Turbo 设置</h3>
        <div class="info-display" style="margin-top: 0; font-size: 12px; margin-bottom: 12px;">设置 Turbo 模式持续时间（1-600秒）</div>
        <div class="turbo-time-input-group">
            <label>Turbo 时间:</label>
            <div class="turbo-time-wrapper">
                <input type="number" id="turboTimeInput" min="1" max="600" value="199" placeholder="1-600">
                <span class="turbo-time-unit">秒</span>
            </div>
        </div>
        <div class="turbo-time-presets">
            <button class="btn btn-primary preset-turbo-btn" data-turbo-time="10">10秒</button>
            <button class="btn btn-primary preset-turbo-btn" data-turbo-time="60">60秒</button>
            <button class="btn btn-primary preset-turbo-btn" data-turbo-time="300">300秒</button>
            <button class="btn btn-primary preset-turbo-btn" data-turbo-time="600">600秒</button>
        </div>
        <div class="turbo-time-buttons">
            <button class="btn btn-success" id="applyTurboTimeBtn">应用设置</button>
            <button class="btn btn-secondary" id="resetTurboTimeBtn">恢复默认</button>
        </div>
        <div class="info-display" id="turboTimeDisplay">当前: 未读取</div>
    </div>
    
    <div class="card">
        <h3>😴 蓝牙休眠设置</h3>
        <div class="info-display" id="autoSleepDisplay">当前: 未设置</div>
        <div class="sleep-delay-input-group">
            <label>自动关闭(秒):</label>
            <div class="sleep-delay-wrapper">
                <input type="number" id="sleepDelayInput" disabled title="米家常在线模式：蓝牙保持永不关闭" min="10" max="65535" value="10" placeholder="10-65535">
                <span class="sleep-delay-unit">秒</span>
            </div>
        </div>
        <div class="sleep-delay-buttons">
            <button class="btn btn-success" id="neverSleepBtn">永不关闭</button>
            <button class="btn btn-warning" id="applySleepDelayBtn" disabled>米家常在线：禁止休眠</button>
        </div>
        <div class="info-display" style="margin-top: 8px; font-size: 12px;">💡 提示: 输入10-65535之间的数值，1-9会自动修正为10</div>
        <!-- 显示SN开关已移至最底部 -->
        <div class="input-group" style="margin-top: 12px;">
            <label>显示SN:</label>
            <button class="btn btn-off" id="bleSnToggleBtn" style="flex: 1; min-width: 120px;">读取中...</button>
        </div>
    </div>
    
    <div class="card">
        <h3>🎛️ 档位风速设置</h3>
        <div class="input-group"><label>1档风速:</label><div class="input-wrapper"><input type="number" id="speed1" min="0" max="100" value="10"><span class="percent-sign">%</span></div></div>
        <div class="input-group"><label>2档风速:</label><div class="input-wrapper"><input type="number" id="speed2" min="0" max="100" value="35"><span class="percent-sign">%</span></div></div>
        <div class="input-group"><label>3档风速:</label><div class="input-wrapper"><input type="number" id="speed3" min="0" max="100" value="70"><span class="percent-sign">%</span></div></div>
        <div class="input-group"><label>4档风速:</label><div class="input-wrapper"><input type="number" id="speed4" min="0" max="100" value="100"><span class="percent-sign">%</span></div></div>
        <div class="button-row">
            <button class="btn btn-primary" id="setSpeedsBtn">应用设置</button>
            <button class="btn btn-secondary" id="resetSpeedsBtn">恢复默认</button>
            <button class="btn btn-success" id="readSpeedsBtn">读取当前</button>
        </div>
        <div class="info-display" id="currentSpeedsDisplay">当前设定: 未读取</div>
    </div>
    
    <div class="card natural-wind">
        <h3>🌬️ 自然风控制</h3>
        <div class="checkbox-group">
            <div class="checkbox-item">
                <input type="checkbox" id="smoothMode" checked>
                <label for="smoothMode">平滑</label>
            </div>
            <div class="checkbox-item">
                <input type="checkbox" id="quietMode">
                <label for="quietMode">安静</label>
            </div>
            <div class="checkbox-item">
                <input type="checkbox" id="strongMode">
                <label for="strongMode">强劲</label>
            </div>
        </div>
        <div class="curve-editor">
            <div class="curve-info">
                <strong>自然风曲线编辑器（十进制）</strong><br>
                共128个数据点（0-100），每个值代表风速强度。<br>
                <strong>点击选择整组数据，编辑后按空格或回车跳转到下一组</strong><br>
                <em>💡 提示：即使自然风关闭，也可以修改和保存曲线，开启后将应用修改后的曲线。</em>
            </div>
            <div class="curve-chart-container">
                <div class="edit-hint">拖动曲线上的点进行调整</div>
                <div class="curve-chart-title">自然风曲线可视化</div>
                <canvas id="curveCanvas" width="400" height="120"></canvas>
                <div class="curve-stats">
                    <span>最小值: <span id="minValue">0</span></span>
                    <span>最大值: <span id="maxValue">100</span></span>
                    <span>平均值: <span id="avgValue">50</span></span>
                </div>
            </div>
            <textarea id="curveData" class="curve-textarea" placeholder="自然风曲线数据（十进制，空格分隔，范围0-100）"></textarea>
            <div class="curve-buttons">
                <button class="btn btn-warning" id="randomCurveBtn">🎲 随机生成曲线</button>
                <button class="btn btn-success" id="applyCurveBtn">🌊 应用编辑的曲线</button>
                <button class="btn btn-secondary" id="restoreCurveBtn">🔄 还原默认曲线</button>
                <button class="btn btn-secondary" id="readCurveBtn">📖 读取自然风曲线</button>
            </div>
        </div>
    </div>
    
    <div class="card">
        <h3>🔋 电池容量设置</h3>
        <div class="input-group"><label>额定容量 (mAh):</label><div class="input-wrapper"><input type="number" id="batteryCapacityInput" min="100" max="50000" value="5000" placeholder="100-50000"><span class="percent-sign">mAh</span></div></div>
        <div class="input-group"><label>额定电压 (V):</label><div class="input-wrapper"><input type="number" id="batteryVoltageInput" min="3" max="30" value="3.6" step="0.1" placeholder="3-30"><span class="percent-sign">V</span></div></div>
        <div class="info-display">计算能量: <span id="calculatedEnergy">--</span> Wh = <span id="calculatedEnergyMWh">--</span> mWh</div>
        <div class="button-row">
            <button class="btn btn-success" id="setBatteryCapacityBtn">设置容量</button>
            <button class="btn btn-secondary" id="readBatteryCapacityBtn">读取容量</button>
        </div>
        <div class="info-display" id="batteryCapacityDisplay">当前容量: 未读取</div>
    </div>
</div>
<div id="toast"></div>

<script>
const SERVICE_DFU = '0000fee0-0000-1000-8000-00805f9b34fb';
const CHAR_WRITE = '0000fee1-0000-1000-8000-00805f9b34fb';
const CHAR_NOTIFY = '0000fee2-0000-1000-8000-00805f9b34fb';
const BLE_CHUNK = 197, DFU_TIMEOUT_MS = 5000;

const CRC8_INIT = 137;
const CRC8_TABLE = [
  0,59,118,77,236,215,154,161,227,216,149,174,15,52,121,66,
  253,198,139,176,17,42,103,92,30,37,104,83,242,201,132,191,
  193,250,183,140,45,22,91,96,34,25,84,111,206,245,184,131,
  60,7,74,113,208,235,166,157,223,228,169,146,51,8,69,126,
  185,130,207,244,85,110,35,24,90,97,44,23,182,141,192,251,
  68,127,50,9,168,147,222,229,167,156,209,234,75,112,61,6,
  120,67,14,53,148,175,226,217,155,160,237,214,119,76,1,58,
  133,190,243,200,105,82,31,36,200,93,16,43,138,177,252,199,
  73,114,63,4,165,158,211,232,170,145,220,231,70,125,48,11,
  180,143,194,249,88,99,46,21,87,108,33,26,187,128,205,246,
  136,179,254,197,100,95,18,41,107,80,29,38,135,188,241,202,
  117,78,3,56,153,162,239,212,150,173,224,219,122,65,12,55,
  240,203,134,189,28,39,106,81,19,40,200,94,255,196,137,178,
  13,54,123,64,225,218,151,172,238,213,152,163,2,57,116,79,
  49,10,71,124,221,230,171,144,210,233,164,159,62,5,72,115,
  204,247,186,129,32,27,86,109,47,20,89,98,195,248,181,142,
];

function calcCrc8(data, off, len) { let crc = CRC8_INIT; for (let i = 0; i < len; i++) crc = CRC8_TABLE[(crc ^ data[off + i]) & 0xff]; return crc & 0xff; }
function dfuPack(payload) {
  const key = 0, frame = new Uint8Array(5 + payload.length);
  frame[0] = 0x55; frame[1] = key; frame[2] = payload.length & 0xff; frame[3] = (payload.length >>> 8) & 0xff; frame.set(payload, 4);
  const xorMask = CRC8_TABLE[key]; for (let i = 2; i < frame.length; i++) frame[i] ^= xorMask;
  frame[frame.length - 1] = calcCrc8(frame, 0, payload.length + 4); return frame;
}
const RxStage = { WAIT_HEAD:0, READ_KEY:1, READ_LEN_LOW:2, READ_LEN_HIGH:3, READ_PAYLOAD:4 };
function makeRx() { return { stage: RxStage.WAIT_HEAD, key:0, len:0, idx:0, crc:CRC8_INIT, buf:new Uint8Array(302) }; }
function onDfuReceive(rx, data, results) {
  for (const raw of data) {
    const b = raw & 0xff;
    switch (rx.stage) {
      case RxStage.WAIT_HEAD: if (b === 0x55) { rx.stage = RxStage.READ_KEY; rx.crc = CRC8_TABLE[(CRC8_INIT ^ 0x55) & 0xff]; } break;
      case RxStage.READ_KEY: rx.key = b; rx.crc = CRC8_TABLE[(rx.crc ^ b) & 0xff]; rx.stage = RxStage.READ_LEN_LOW; break;
      case RxStage.READ_LEN_LOW: { const d = b ^ CRC8_TABLE[rx.key]; rx.len = d; rx.crc = CRC8_TABLE[(rx.crc ^ d) & 0xff]; rx.stage = RxStage.READ_LEN_HIGH; break; }
      case RxStage.READ_LEN_HIGH: { const d = b ^ CRC8_TABLE[rx.key]; rx.len |= d << 8; rx.crc = CRC8_TABLE[(rx.crc ^ d) & 0xff]; if (rx.len < 1 || rx.len > 300) { rx.stage = RxStage.WAIT_HEAD; break; } rx.idx = 0; rx.stage = RxStage.READ_PAYLOAD; break; }
      case RxStage.READ_PAYLOAD: { const d = b ^ CRC8_TABLE[rx.key]; rx.buf[rx.idx] = d; if (rx.idx < rx.len) rx.crc = CRC8_TABLE[(rx.crc ^ d) & 0xff]; rx.idx++; if (rx.idx === rx.len + 1) { if (rx.crc === d) results.push(rx.buf.slice(0, rx.len)); rx.stage = RxStage.WAIT_HEAD; } break; }
    }
  }
}

const CONFIG = Object.freeze({
    MAIN_SERVICE_UUID: "0000fff0-0000-1000-8000-00805f9b34fb",
    FFD0_SERVICE_UUID: "0000ffd0-0000-1000-8000-00805f9b34fb",
    FFC0_SERVICE_UUID: "0000ffc0-0000-1000-8000-00805f9b34fb",
    NATURE_WIND_SERVICE_UUID: "0000ffe0-0000-1000-8000-00805f9b34fb",
    CHARACTERISTICS: {
        POWER: "0000fff1-0000-1000-8000-00805f9b34fb",
        TIMER: "0000fff2-0000-1000-8000-00805f9b34fb",
        FAN_SPEED: "0000fff3-0000-1000-8000-00805f9b34fb",
        NATURE_WIND: "0000fff4-0000-1000-8000-00805f9b34fb",
        SHUTDOWN_DELAY: "0000fff5-0000-1000-8000-00805f9b34fb",
        GEAR_DOWN_MODE: "0000fff6-0000-1000-8000-00805f9b34fb",
        SPEED_CALIB: "0000fff7-0000-1000-8000-00805f9b34fb",
        BATTERY_INFO: "0000ffd1-0000-1000-8000-00805f9b34fb",
        POWER_STATUS: "0000ffd2-0000-1000-8000-00805f9b34fb",
        MOTOR_INFO: "0000ffd3-0000-1000-8000-00805f9b34fb",
        POWER_CONFIG: "0000ffd4-0000-1000-8000-00805f9b34fb",
        NATURE_WIND_CURVE: "0000ffe3-0000-1000-8000-00805f9b34fb",
        TURBO_MODE: "0000fffc-0000-1000-8000-00805f9b34fb",
        TURBO_REMAINING: "0000fffb-0000-1000-8000-00805f9b34fb",
        TURBO_TIME: "0000fff8-0000-1000-8000-00805f9b34fb",
        LIGHT_CONTROL: "0000fffa-0000-1000-8000-00805f9b34fb",
        BLE_SN_CONTROL: "0000ffc1-0000-1000-8000-00805f9b34fb"
    },
    DEFAULT_SPEEDS: [10, 35, 70, 100],
    DEFAULT_FAN_SPEED: 50,
    MIN_SPEED_LIMIT: 0, MAX_SPEED_LIMIT: 100,
    DEFAULT_NATURE_WIND_CURVE_DECIMAL: "55 48 40 33 28 22 21 26 33 41 48 54 58 60 61 58 52 45 37 30 24 20 25 33 40 48 53 57 60 60 56 51 43 36 29 23 21 28 37 47 56 63 68 71 72 71 67 62 54 46 36 29 23 20 27 37 48 57 64 69 73 74 76 78 80 82 84 86 88 90 89 87 83 77 70 62 53 43 34 27 21 20 26 32 38 43 47 49 50 48 44 38 33 27 24 20 21 26 31 37 42 46 48 47 42 36 31 27 23 20 22 27 33 39 44 47 48 46 41 36 30 26 23 20 22 27 33 38"
});

const state = {
    device: null, mainService: null, ffd0Service: null, ffc0Service: null, natureWindService: null,
    dfuService: null, dfuWriteChar: null, dfuNotifyChar: null,
    naturalWindOn: false, turboOn: false, currentTurboTime: 199, currentLightBrightness: null,
    gearDownMode: 0, currentPower: '00', disconnectListener: null, currentFanSpeed: CONFIG.DEFAULT_FAN_SPEED,
    curveData: [], selectedPointIndex: -1, isDragging: false, refreshInterval: null,
    batteryCapacity: 17200, autoSleepDelay: 0, bleSnEnabled: null,
    lastVbusVoltage: 0, lastVbusCurrent: 0, lastBatteryCurrent: 0, lastBatteryVoltage: 0,
    lastMotorCurrent: 0, lastMotorVoltage: 0, chargeStatus: 0,
    isConnecting: false, isReadingDeviceInfo: false, cachedChars: new Map(), pageHidden: false,
    isPowerSending: false, pendingPowerHex: null, isCurveOperationInProgress: false, isBatteryOperationInProgress: false,
    speedAdjustInterval: null, speedAdjustTimeout: null, isAdjustingSpeed: false, isManualInputFocused: false,
    ignoreNextSpeedUpdate: false, speedChangeInProgress: false, isSleepDelayInputFocused: false, sleepDelayWriteInProgress: false
};

let modeToggleLock = false;

const elements = {
    status: document.getElementById('status'), serialNumberValue: document.getElementById('serialNumberValue'), firmwareVersionValue: document.getElementById('firmwareVersionValue'),
    refreshDeviceInfoBtn: document.getElementById('refreshDeviceInfoBtn'), turboBtn: document.getElementById('turboBtn'), nwBtn: document.getElementById('nwBtn'),
    gearDownModeBtn: document.getElementById('gearDownModeBtn'), lightStatusDisplay: document.getElementById('lightStatusDisplay'),
    turboTimeInput: document.getElementById('turboTimeInput'), turboTimeDisplay: document.getElementById('turboTimeDisplay'),
    remainingTime: document.getElementById('remainingTimeDisplay'), currentSpeeds: document.getElementById('currentSpeedsDisplay'),
    customMinutes: document.getElementById('customMinutes'),
    speedInputs: [document.getElementById('speed1'), document.getElementById('speed2'), document.getElementById('speed3'), document.getElementById('speed4')],
    speedSlider: document.getElementById('speedSlider'), currentSpeedValue: document.getElementById('currentSpeedValue'),
    manualSpeedInput: document.getElementById('manualSpeedInput'),
    decreaseSpeedBtn: document.getElementById('decreaseSpeedBtn'), increaseSpeedBtn: document.getElementById('increaseSpeedBtn'),
    curveData: document.getElementById('curveData'), curveCanvas: document.getElementById('curveCanvas'),
    minValue: document.getElementById('minValue'), maxValue: document.getElementById('maxValue'), avgValue: document.getElementById('avgValue'),
    smoothMode: document.getElementById('smoothMode'), quietMode: document.getElementById('quietMode'), strongMode: document.getElementById('strongMode'),
    fanSpeedValue: document.getElementById('fanSpeedValue'), vbusVoltageValue: document.getElementById('vbusVoltageValue'), vbusCurrentValue: document.getElementById('vbusCurrentValue'),
    batteryCurrentValue: document.getElementById('batteryCurrentValue'), chargeStatusValue: document.getElementById('chargeStatusValue'),
    batteryVoltageValue: document.getElementById('batteryVoltageValue'), powerValue: document.getElementById('powerValue'),
    motorCurrentValue: document.getElementById('motorCurrentValue'), motorPowerValue: document.getElementById('motorPowerValue'), motorVoltageValue: document.getElementById('motorVoltageValue'),
    batteryCapacityInput: document.getElementById('batteryCapacityInput'), batteryVoltageInput: document.getElementById('batteryVoltageInput'),
    calculatedEnergy: document.getElementById('calculatedEnergy'), calculatedEnergyMWh: document.getElementById('calculatedEnergyMWh'),
    batteryCapacityDisplay: document.getElementById('batteryCapacityDisplay'),
    autoSleepDisplay: document.getElementById('autoSleepDisplay'), sleepDelayInput: document.getElementById('sleepDelayInput'),
    neverSleepBtn: document.getElementById('neverSleepBtn'), applySleepDelayBtn: document.getElementById('applySleepDelayBtn'),
    setBatteryCapacityBtn: document.getElementById('setBatteryCapacityBtn'), readBatteryCapacityBtn: document.getElementById('readBatteryCapacityBtn'),
    bleSnToggleBtn: document.getElementById('bleSnToggleBtn')
};

const utils = {
    decimalToHex: (d) => parseInt(d).toString(16).padStart(2, '0').toUpperCase(),
    hexToDecimal: (h) => parseInt(h, 16),
    decimalArrayToHexString: (arr) => arr.map(v => utils.decimalToHex(v)).join(''),
    readUInt16BE: (dv, off) => { if (dv.byteLength < off + 2) return 0; return dv.getUint16(off, false); },
    readInt16BE: (dv, off) => { if (dv.byteLength < off + 2) return 0; return dv.getInt16(off, false); },
    readUInt32BE: (dv, off) => { if (dv.byteLength < off + 4) return 0; return dv.getUint32(off, false); },
    showToast: (msg, duration = 2000) => { const toast = document.getElementById('toast'); toast.textContent = msg; toast.classList.add('show'); setTimeout(() => toast.classList.remove('show'), duration); },
    alert: (msg) => { alert(msg); },
    parseBatteryInfo: (dataView) => ({ voltage: utils.readUInt16BE(dataView, 0), current: utils.readInt16BE(dataView, 2), capacity: utils.readUInt32BE(dataView, 4) }),
    parsePowerStatus: (dataView) => { const vbusV = utils.readUInt32BE(dataView, 0), vbusCur = utils.readInt16BE(dataView, 4); return { vbusVoltage: vbusV, vbusCurrent: vbusCur === 0x7FFF ? 0 : vbusCur, chargeStatus: dataView.getUint8(7) }; },
    parseMotorInfo: (dataView) => { const motCur = utils.readUInt16BE(dataView, 0), motorBlock = dataView.getUint8(2); let motorVol = 0; if (dataView.byteLength >= 2) motorVol = utils.readUInt16BE(dataView, dataView.byteLength - 2); return { current: 0, block: motorBlock, voltage: motorVol, rawCurrent: motCur }; },
    formatVoltage: (mv) => `${(mv / 1000).toFixed(2)} V`,
    formatCurrent: (ma) => Math.abs(ma) >= 1000 ? `${(ma / 1000).toFixed(2)} A` : `${ma} mA`,
    formatPower: (power) => `${power.toFixed(2)} W`,
};

function hexStringToBytes(hex) { const bytes = []; for (let i = 0; i < hex.length; i += 2) bytes.push(parseInt(hex.substr(i, 2), 16)); return new Uint8Array(bytes); }
function speedToHex(s) { return s.toString(16).padStart(2, '0').toUpperCase(); }

async function dfuRequest(payload) {
    if (!state.dfuWriteChar || !state.dfuNotifyChar) throw new Error("DFU 服务未就绪");
    const rx = makeRx(), results = [];
    const handler = (e) => { if (e.target.value) onDfuReceive(rx, new Uint8Array(e.target.value.buffer, e.target.value.byteOffset, e.target.value.byteLength), results); };
    state.dfuNotifyChar.addEventListener('characteristicvaluechanged', handler);
    try {
        results.length = 0; const frame = dfuPack(payload);
        for (let off = 0; off < frame.length; off += BLE_CHUNK) await state.dfuWriteChar.writeValueWithoutResponse(frame.slice(off, off + BLE_CHUNK));
        const startTime = Date.now(); while (results.length === 0) { if (Date.now() - startTime > DFU_TIMEOUT_MS) throw new Error('DFU 请求超时'); await new Promise(r => setTimeout(r, 50)); }
        return results[0];
    } finally { state.dfuNotifyChar.removeEventListener('characteristicvaluechanged', handler); }
}



async function safeWrite(characteristicKey, hexValue, retry = true) {
    if (!state.mainService) { utils.alert("蓝牙服务未就绪"); return false; }
    const uuid = CONFIG.CHARACTERISTICS[characteristicKey]; if (!uuid) { utils.alert(`未知特征: ${characteristicKey}`); return false; }
    try {
        let char = state.cachedChars.get(uuid); if (!char) { char = await state.mainService.getCharacteristic(uuid); state.cachedChars.set(uuid, char); }
        await char.writeValue(hexStringToBytes(hexValue)); return true;
    } catch (e) { if (retry) { await new Promise(r => setTimeout(r, 200)); return safeWrite(characteristicKey, hexValue, false); } utils.alert(e.message || '写入未确认'); return false; }
}

async function writeAsciiCommand(characteristicKey, command) {
    if (!state.ffd0Service) { utils.alert("电源服务未就绪"); return false; }
    const uuid = CONFIG.CHARACTERISTICS[characteristicKey]; if (!uuid) { utils.alert(`未知特征: ${characteristicKey}`); return false; }
    try { let char = state.cachedChars.get(uuid); if (!char) { char = await state.ffd0Service.getCharacteristic(uuid); state.cachedChars.set(uuid, char); } await char.writeValue(new TextEncoder().encode(command)); return true; } catch (e) { utils.alert(`写入失败: ${e.message}`); return false; }
}

async function writeFfc0AsciiCommand(characteristicKey, command) {
    if (!state.ffc0Service) { utils.alert("FFC0服务未就绪"); return false; }
    const uuid = CONFIG.CHARACTERISTICS[characteristicKey]; if (!uuid) { utils.alert(`未知特征: ${characteristicKey}`); return false; }
    try { let char = state.cachedChars.get(uuid); if (!char) { char = await state.ffc0Service.getCharacteristic(uuid); state.cachedChars.set(uuid, char); } await char.writeValue(new TextEncoder().encode(command)); return true; } catch (e) { utils.alert(`写入失败: ${e.message}`); return false; }
}

async function ensureTurboOff() {
    if (!state.turboOn) return;
    if (!await safeWrite('TURBO_MODE', '00')) throw new Error('关闭 Turbo 失败'); state.turboOn = false;
    elements.turboBtn.textContent = 'Turbo OFF'; elements.turboBtn.className = 'btn btn-off turbo-btn';
    utils.showToast("Turbo 模式已自动关闭");
}

async function setPower(hexValue) {
    if (!state.device?.gatt.connected) { utils.alert("未连接"); return false; }
    if (state.isPowerSending) { state.pendingPowerHex = hexValue; return true; }
    state.isPowerSending = true;
    try {
        await ensureTurboOff();
        if (state.naturalWindOn) { if (!await safeWrite('NATURE_WIND', '00')) throw new Error('关闭自然风失败'); state.naturalWindOn = false; elements.nwBtn.textContent = '自然风 OFF'; elements.nwBtn.className = 'btn btn-off'; await new Promise(r => setTimeout(r, 100)); }
        if (!await safeWrite('POWER', hexValue, true)) return false; state.currentPower = hexValue;
    } finally { state.isPowerSending = false; if (state.pendingPowerHex !== null) { const next = state.pendingPowerHex; state.pendingPowerHex = null; await setPower(next); } }
    return true;
}

async function sendSpeedImmediately(speed) {
    if (!state.device?.gatt.connected) return;
    state.bridgePendingSpeed=speed;
    if (state.bridgeSpeedPromise) return state.bridgeSpeedPromise;
    state.speedChangeInProgress=true;
    state.bridgeSpeedPromise=(async()=>{
      while(state.bridgePendingSpeed!==null && state.device?.gatt.connected){
        const target=state.bridgePendingSpeed;state.bridgePendingSpeed=null;
        await ensureTurboOff();
        if(state.currentPower==='00'){if(!await safeWrite('POWER','01'))throw new Error('开启风扇失败');state.currentPower='01';}
        if(state.naturalWindOn){if(!await safeWrite('NATURE_WIND','00'))throw new Error('关闭自然风失败');state.naturalWindOn=false;elements.nwBtn.textContent='自然风 OFF';elements.nwBtn.className='btn btn-off';}
        const c=await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.FAN_SPEED);
        await c.writeValue(new Uint8Array([target]));
      }
    })();
    try{await state.bridgeSpeedPromise;}
    catch(e){state.bridgePendingSpeed=null;utils.alert(e.message || '风速设置失败');throw e;}
    finally{state.bridgeSpeedPromise=null;state.speedChangeInProgress=false;}
}

function onSpeedSliderChange(value) {
    let speed = Math.min(100, Math.max(0, parseInt(value) || 0));
    state.currentFanSpeed = speed; elements.currentSpeedValue.textContent = `${speed}%`;
    if (!state.isManualInputFocused) elements.manualSpeedInput.value = speed;
    elements.fanSpeedValue.textContent = `${speed} %`;
    if (state.isSliderBeingDragged) sendSpeedImmediately(speed);
}

function adjustSpeed(delta) {
    let newSpeed = Math.min(100, Math.max(0, state.currentFanSpeed + delta));
    if (newSpeed !== state.currentFanSpeed) {
        state.currentFanSpeed = newSpeed; elements.speedSlider.value = newSpeed; elements.currentSpeedValue.textContent = `${newSpeed}%`;
        if (!state.isManualInputFocused) elements.manualSpeedInput.value = newSpeed;
        elements.fanSpeedValue.textContent = `${newSpeed} %`; sendSpeedImmediately(newSpeed);
    }
}

function startContinuousAdjust(isIncrease) {
    if (state.isAdjustingSpeed) return;
    state.isAdjustingSpeed = true; pauseRealTimeRefreshCompletely(); adjustSpeed(isIncrease ? 1 : -1);
    state.speedAdjustTimeout = setTimeout(() => { state.speedAdjustInterval = setInterval(() => adjustSpeed(isIncrease ? 1 : -1), 80); }, 150);
}

function stopContinuousAdjust() {
    state.isAdjustingSpeed = false;
    if (state.speedAdjustTimeout) { clearTimeout(state.speedAdjustTimeout); state.speedAdjustTimeout = null; }
    if (state.speedAdjustInterval) { clearInterval(state.speedAdjustInterval); state.speedAdjustInterval = null; }
    setTimeout(() => { state.ignoreNextSpeedUpdate = true; resumeRealTimeRefresh(); }, 300);
}

async function validateAndApplySpeedFromInput() {
    let speed = parseInt(elements.manualSpeedInput.value.trim());
    if (isNaN(speed) || speed < 0 || speed > 100) { utils.alert("转速请输入0-100之间的数值！"); return; }
    state.ignoreNextSpeedUpdate = true; state.currentFanSpeed = speed;
    elements.speedSlider.value = speed; elements.currentSpeedValue.textContent = `${speed}%`;
    elements.manualSpeedInput.value = speed; elements.fanSpeedValue.textContent = `${speed} %`;
    await sendSpeedImmediately(speed); utils.showToast(`转速已设置为${speed}%`);
}

let realTimeRefreshPaused = false, realTimeRefreshForcePaused = false;
function pauseRealTimeRefresh() { realTimeRefreshPaused = true; stopRealTimeRefresh(); }
function pauseRealTimeRefreshCompletely() { realTimeRefreshForcePaused = true; realTimeRefreshPaused = true; stopRealTimeRefresh(); }
function resumeRealTimeRefresh() { realTimeRefreshPaused = false; realTimeRefreshForcePaused = false; startRealTimeRefresh(); }

async function refreshSleepDelayImmediately() {
    if (!state.device?.gatt.connected || !state.mainService) return;
    try {
        let sleepChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY);
        if (!sleepChar) { sleepChar = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY); state.cachedChars.set(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY, sleepChar); }
        const val = await sleepChar.readValue(); if (val.byteLength >= 2) { const delay = val.getUint16(0, false); state.autoSleepDelay = delay; if (delay === 0) { elements.autoSleepDisplay.textContent = "当前: 永不关闭"; if (!state.isSleepDelayInputFocused) elements.sleepDelayInput.value = 10; } else { elements.autoSleepDisplay.textContent = `当前: ${delay}秒后自动关闭`; if (!state.isSleepDelayInputFocused) elements.sleepDelayInput.value = delay; } }
    } catch(e) { console.warn("刷新休眠设置失败:", e); }
}

let bridgeRefreshing=false;
async function vendorRefreshRealTimeStatus() {
    if (!state.device?.gatt.connected || state.pageHidden || realTimeRefreshPaused || realTimeRefreshForcePaused) return;
    if (state.ignoreNextSpeedUpdate) { state.ignoreNextSpeedUpdate = false; return; }
    try {
        if (state.mainService) {
            try {
                let fanSpeedChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.FAN_SPEED); if (!fanSpeedChar) { fanSpeedChar = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.FAN_SPEED); state.cachedChars.set(CONFIG.CHARACTERISTICS.FAN_SPEED, fanSpeedChar); }
                const val = await fanSpeedChar.readValue(); const speed = val.getUint8(0);
                if (!state.isManualInputFocused && !state.isAdjustingSpeed) { elements.fanSpeedValue.textContent = `${speed} %`; state.currentFanSpeed = speed; elements.speedSlider.value = speed; elements.currentSpeedValue.textContent = `${speed}%`; elements.manualSpeedInput.value = speed; }
            } catch(e) { console.warn("读取转速失败:", e); }
        }
        if (state.ffd0Service) {
            try {
                let batInfoChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.BATTERY_INFO); if (!batInfoChar) { batInfoChar = await state.ffd0Service.getCharacteristic(CONFIG.CHARACTERISTICS.BATTERY_INFO); state.cachedChars.set(CONFIG.CHARACTERISTICS.BATTERY_INFO, batInfoChar); }
                const batView = new DataView((await batInfoChar.readValue()).buffer), bi = utils.parseBatteryInfo(batView);
                elements.batteryVoltageValue.textContent = utils.formatVoltage(bi.voltage); state.lastBatteryVoltage = bi.voltage / 1000;
                elements.batteryCurrentValue.textContent = utils.formatCurrent(bi.current); state.lastBatteryCurrent = bi.current;
                elements.batteryCapacityDisplay.textContent = `当前容量: ${bi.capacity} mWh`; state.batteryCapacity = bi.capacity;
            } catch(e) { console.warn("读取电池信息失败:", e); }
        }
        if (state.ffd0Service) {
            try {
                let powerChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.POWER_STATUS); if (!powerChar) { powerChar = await state.ffd0Service.getCharacteristic(CONFIG.CHARACTERISTICS.POWER_STATUS); state.cachedChars.set(CONFIG.CHARACTERISTICS.POWER_STATUS, powerChar); }
                const pv = new DataView((await powerChar.readValue()).buffer), pi = utils.parsePowerStatus(pv);
                elements.vbusVoltageValue.textContent = utils.formatVoltage(pi.vbusVoltage); state.lastVbusVoltage = pi.vbusVoltage;
                elements.vbusCurrentValue.textContent = utils.formatCurrent(pi.vbusCurrent); state.lastVbusCurrent = pi.vbusCurrent;
                elements.chargeStatusValue.textContent = pi.chargeStatus === 1 ? '充电中' : '放电中'; elements.chargeStatusValue.style.color = pi.chargeStatus === 1 ? '#00b894' : '#ff6b6b'; state.chargeStatus = pi.chargeStatus;
            } catch(e) { console.warn("读取电源状态失败:", e); }
        }
        if (state.ffd0Service) {
            try {
                let motorInfoChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.MOTOR_INFO); if (!motorInfoChar) { motorInfoChar = await state.ffd0Service.getCharacteristic(CONFIG.CHARACTERISTICS.MOTOR_INFO); state.cachedChars.set(CONFIG.CHARACTERISTICS.MOTOR_INFO, motorInfoChar); }
                const mv = new DataView((await motorInfoChar.readValue()).buffer), mi = utils.parseMotorInfo(mv);
                elements.motorCurrentValue.textContent = utils.formatCurrent(mi.rawCurrent); state.lastMotorCurrent = mi.rawCurrent;
                if (mi.voltage !== undefined) { elements.motorVoltageValue.textContent = utils.formatVoltage(mi.voltage); state.lastMotorVoltage = mi.voltage; }
                if (state.lastMotorCurrent !== undefined && state.lastMotorVoltage !== undefined) { const mp = (state.lastMotorVoltage / 1000) * (state.lastMotorCurrent / 1000); elements.motorPowerValue.textContent = utils.formatPower(mp); elements.motorPowerValue.style.color = '#00b894'; }
            } catch(e) { console.warn("读取电机信息失败:", e); }
        }
        if (state.lastBatteryVoltage && state.lastBatteryCurrent !== undefined) { const p = state.lastBatteryVoltage * (state.lastBatteryCurrent / 1000); elements.powerValue.textContent = utils.formatPower(p); elements.powerValue.style.color = p >= 0 ? '#00b894' : '#ff6b6b'; }
        if (state.mainService) {
            try {
                let turboRemChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.TURBO_REMAINING); if (!turboRemChar) { turboRemChar = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.TURBO_REMAINING); state.cachedChars.set(CONFIG.CHARACTERISTICS.TURBO_REMAINING, turboRemChar); }
                const val = await turboRemChar.readValue(); if (val.byteLength >= 2) { const rem = val.getUint16(0, false); if (rem > 0) { state.turboOn = true; elements.turboBtn.textContent = `Turbo ${rem}s ON`; elements.turboBtn.className = 'btn btn-turbo-on turbo-btn'; } else if (state.turboOn) { state.turboOn = false; elements.turboBtn.textContent = 'Turbo OFF'; elements.turboBtn.className = 'btn btn-off turbo-btn'; } }
            } catch(e) { console.warn("读取Turbo倒计时失败:", e); }
        }
        if (state.mainService) {
            try {
                let nwChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.NATURE_WIND); if (!nwChar) { nwChar = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.NATURE_WIND); state.cachedChars.set(CONFIG.CHARACTERISTICS.NATURE_WIND, nwChar); }
                const val = await nwChar.readValue(); const isOn = val.getUint8(0) === 1; if (state.naturalWindOn !== isOn) { state.naturalWindOn = isOn; elements.nwBtn.textContent = isOn ? '自然风 ON ' : '自然风 OFF'; elements.nwBtn.className = isOn ? 'btn btn-on' : 'btn btn-off'; }
            } catch(e) { console.warn("读取自然风状态失败:", e); }
        }
        if (state.mainService) {
            try {
                let gearDownChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.GEAR_DOWN_MODE); if (!gearDownChar) { gearDownChar = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.GEAR_DOWN_MODE); state.cachedChars.set(CONFIG.CHARACTERISTICS.GEAR_DOWN_MODE, gearDownChar); }
                const val = await gearDownChar.readValue(); const mode = val.getUint8(0); state.gearDownMode = mode;
                if (mode === 0) { elements.gearDownModeBtn.textContent = '减档: 逐级减档'; elements.gearDownModeBtn.className = 'btn btn-secondary gear-down-mode-btn'; } else if (mode === 1) { elements.gearDownModeBtn.textContent = '减档: 直接回0'; elements.gearDownModeBtn.className = 'btn btn-warning gear-down-mode-btn'; } else { elements.gearDownModeBtn.textContent = '减档: 未知状态'; elements.gearDownModeBtn.className = 'btn btn-danger gear-down-mode-btn'; }
            } catch(e) { console.warn("读取减档模式失败:", e); }
        }
        if (state.mainService && !state.isSleepDelayInputFocused && !state.sleepDelayWriteInProgress) {
            try {
                let sleepChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY); if (!sleepChar) { sleepChar = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY); state.cachedChars.set(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY, sleepChar); }
                const val = await sleepChar.readValue(); if (val.byteLength >= 2) { const delay = val.getUint16(0, false); state.autoSleepDelay = delay; if (delay === 0) { elements.autoSleepDisplay.textContent = "当前: 永不关闭"; elements.sleepDelayInput.value = 10; } else { elements.autoSleepDisplay.textContent = `当前: ${delay}秒后自动关闭`; elements.sleepDelayInput.value = delay; } }
            } catch(e) { console.warn("读取休眠设置失败:", e); }
        }
    } catch(e) { console.warn("实时状态刷新错误:", e); }
}

async function refreshRealTimeStatus(){if(bridgeRefreshing)return;bridgeRefreshing=true;try{await vendorRefreshRealTimeStatus();}finally{bridgeRefreshing=false;}}
function startRealTimeRefresh() { if (state.refreshInterval) clearInterval(state.refreshInterval); refreshRealTimeStatus(); state.refreshInterval = setInterval(refreshRealTimeStatus, 1000); }
function stopRealTimeRefresh() { if (state.refreshInterval) { clearInterval(state.refreshInterval); state.refreshInterval = null; } }

document.addEventListener('visibilitychange', () => { state.pageHidden = document.hidden; if (state.pageHidden) stopRealTimeRefresh(); else if (state.device?.gatt.connected && !realTimeRefreshPaused && !realTimeRefreshForcePaused) startRealTimeRefresh(); });

async function readLightStatus() {
    if (!state.mainService) return;
    try {
        let char = state.cachedChars.get(CONFIG.CHARACTERISTICS.LIGHT_CONTROL); if (!char) { char = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.LIGHT_CONTROL); state.cachedChars.set(CONFIG.CHARACTERISTICS.LIGHT_CONTROL, char); }
        const val = await char.readValue(); if (val.byteLength >= 1) { const hv = val.getUint8(0).toString(16).padStart(2,'0'), map = {'00':'关灯','01':'低亮度','02':'中低亮度','03':'中高亮度','04':'最高亮度'}; state.currentLightBrightness = hv; elements.lightStatusDisplay.textContent = `当前: ${map[hv]||'未知'}`; document.querySelectorAll('.light-btn').forEach(btn => btn.classList.toggle('btn-on', btn.dataset.brightness === hv)); }
    } catch(e) { console.warn("读取灯光状态失败:", e); }
}

function updateBleSnButton() {
    const btn = elements.bleSnToggleBtn;
    if (state.bleSnEnabled === null) {
        btn.textContent = '未读取';
        btn.className = 'btn btn-secondary';
    } else if (state.bleSnEnabled) {
        btn.textContent = '显示SN ON';
        btn.className = 'btn btn-success';
    } else {
        btn.textContent = '显示SN OFF';
        btn.className = 'btn btn-off';
    }
}

async function toggleBleSn() {
    if (!state.device?.gatt.connected) { utils.alert("请先连接风扇"); return; }
    if (modeToggleLock) { utils.showToast("请稍候..."); return; }
    
    modeToggleLock = true;
    const wantOn = !state.bleSnEnabled;
    
    try {
        const cmd = wantOn ? 'BLE_SN=1,' : 'BLE_SN=0,';
        const ok = await writeFfc0AsciiCommand('BLE_SN_CONTROL', cmd);
        if (ok) {
            state.bleSnEnabled = wantOn;
            updateBleSnButton();
            utils.showToast(wantOn ? '已开启显示序列号' : '已关闭显示序列号');
        } else {
            utils.alert('切换失败');
        }
    } finally {
        modeToggleLock = false;
    }
}

async function readBleSnStatus() {
    if (!state.ffc0Service) return;
    try {
        let char = state.cachedChars.get(CONFIG.CHARACTERISTICS.BLE_SN_CONTROL);
        if (!char) { char = await state.ffc0Service.getCharacteristic(CONFIG.CHARACTERISTICS.BLE_SN_CONTROL); state.cachedChars.set(CONFIG.CHARACTERISTICS.BLE_SN_CONTROL, char); }
        const val = await char.readValue();
        const str = new TextDecoder().decode(val);
        state.bleSnEnabled = str.includes('BLE_SN=1');
        updateBleSnButton();
    } catch(e) {
        console.warn("读取BLE SN状态失败:", e);
        state.bleSnEnabled = null;
        updateBleSnButton();
    }
}



async function readNatureWindStatusWithRetry(retries = 3) {
    if (!state.mainService) return;
    while (retries > 0) { try { let char = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.NATURE_WIND); state.cachedChars.set(CONFIG.CHARACTERISTICS.NATURE_WIND, char); const val = await char.readValue(); const isOn = val.getUint8(0) === 1; state.naturalWindOn = isOn; elements.nwBtn.textContent = isOn ? '自然风 ON ' : '自然风 OFF'; elements.nwBtn.className = isOn ? 'btn btn-on' : 'btn btn-off'; return; } catch(e) { retries--; if (retries === 0) { state.naturalWindOn = false; elements.nwBtn.textContent = '自然风 OFF'; elements.nwBtn.className = 'btn btn-off'; } else await new Promise(r => setTimeout(r, 300)); } }
}

async function readTurboTime() {
    if (!state.mainService) return;
    try { let char = state.cachedChars.get(CONFIG.CHARACTERISTICS.TURBO_TIME); if (!char) { char = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.TURBO_TIME); state.cachedChars.set(CONFIG.CHARACTERISTICS.TURBO_TIME, char); } const val = await char.readValue(); if (val.byteLength >= 2) { const secs = val.getUint16(0, false); state.currentTurboTime = secs; if (secs === 0) { elements.turboTimeDisplay.textContent = "当前: 199 秒"; elements.turboTimeInput.value = 199; } else { elements.turboTimeDisplay.textContent = `当前: ${secs} 秒`; elements.turboTimeInput.value = secs; } } } catch(e) { elements.turboTimeDisplay.textContent = "当前: 读取失败"; }
}

async function setTurboTime(seconds) {
    if (!state.device?.gatt.connected) { utils.alert("请先连接风扇"); return false; }
    let secs = parseInt(seconds); if (isNaN(secs) || secs < 0 || secs > 600) { utils.alert("Turbo 时间范围为 0-600 秒"); return false; }
    const hexValue = secs.toString(16).padStart(4, '0').toUpperCase(); const success = await safeWrite('TURBO_TIME', hexValue);
    if (success) { state.currentTurboTime = secs; if (secs === 0) { elements.turboTimeDisplay.textContent = "当前: 199 秒"; elements.turboTimeInput.value = 199; utils.showToast("已恢复默认 Turbo 时间 (199秒)"); } else { elements.turboTimeDisplay.textContent = `当前: ${secs} 秒`; elements.turboTimeInput.value = secs; utils.showToast(`Turbo 时间已设置为 ${secs} 秒`); } } return success;
}

async function readGearDownModeWithRetry(retries = 3) {
    if (!state.mainService) return;
    while (retries > 0) { try { let char = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.GEAR_DOWN_MODE); state.cachedChars.set(CONFIG.CHARACTERISTICS.GEAR_DOWN_MODE, char); const val = await char.readValue(); const mode = val.getUint8(0); state.gearDownMode = mode; if (mode === 0) { elements.gearDownModeBtn.textContent = '减档: 逐级减档'; elements.gearDownModeBtn.className = 'btn btn-secondary gear-down-mode-btn'; } else if (mode === 1) { elements.gearDownModeBtn.textContent = '减档: 直接回0'; elements.gearDownModeBtn.className = 'btn btn-warning gear-down-mode-btn'; } else { elements.gearDownModeBtn.textContent = '减档: 未知状态'; elements.gearDownModeBtn.className = 'btn btn-danger gear-down-mode-btn'; } return; } catch(e) { retries--; if (retries === 0) { elements.gearDownModeBtn.textContent = '减档: 读取失败'; elements.gearDownModeBtn.className = 'btn btn-danger gear-down-mode-btn'; } else await new Promise(r => setTimeout(r, 300)); } }
}

async function readAutoSleepSettingWithRetry(retries = 5) {
    if (!state.mainService) return;
    while (retries > 0) { try { let char = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY); state.cachedChars.set(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY, char); await new Promise(r => setTimeout(r, 100)); const val = await char.readValue(); if (val.byteLength < 2) throw new Error(`数据长度不足`); const delay = val.getUint16(0, false); state.autoSleepDelay = delay; if (delay === 0) { elements.autoSleepDisplay.textContent = "当前: 永不关闭"; elements.sleepDelayInput.value = 10; } else { elements.autoSleepDisplay.textContent = `当前: ${delay}秒后自动关闭`; elements.sleepDelayInput.value = delay; } return; } catch(e) { retries--; if (retries === 0) elements.autoSleepDisplay.textContent = "当前: 读取失败"; else await new Promise(r => setTimeout(r, 400)); } }
}

async function readCurrentSpeedsWithRetry(retries = 3) {
    if (!state.mainService) return;
    while (retries > 0) {
        try {
            let char = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.SPEED_CALIB); state.cachedChars.set(CONFIG.CHARACTERISTICS.SPEED_CALIB, char);
            const value = await char.readValue(); if (value.byteLength < 4) throw new Error(`数据长度不足`);
            const speeds = []; for (let i = 0; i < 4; i++) speeds.push(value.getUint8(i));
            speeds.forEach((s, i) => { if (elements.speedInputs[i]) { elements.speedInputs[i].value = s; elements.speedInputs[i].style.borderColor = '#e9ecef'; } });
            elements.currentSpeeds.textContent = `当前设定: ${speeds.join('% / ')}%`;
            utils.showToast('档位风速已读取'); return;
        } catch(e) { retries--; if (retries === 0) elements.currentSpeeds.textContent = "当前设定: 读取失败"; else await new Promise(r => setTimeout(r, 300)); }
    }
}

async function setLightBrightness(hexValue) {
    if (!state.device?.gatt.connected) { utils.alert("请先连接风扇"); return; }
    if (!await safeWrite('LIGHT_CONTROL', hexValue)) return;
    const map = {'00':'关灯','01':'低亮度','02':'中低亮度','03':'中高亮度','04':'最高亮度'};
    state.currentLightBrightness = hexValue; elements.lightStatusDisplay.textContent = `当前: ${map[hexValue]||'未知'}`;
    document.querySelectorAll('.light-btn').forEach(btn => btn.classList.toggle('btn-on', btn.dataset.brightness === hexValue));
    utils.showToast(`灯光已设置为${map[hexValue]}`);
}

async function toggleTurbo() {
    if (!state.device?.gatt.connected) { utils.alert("请先连接风扇"); return; }
    if (modeToggleLock) { utils.showToast("请稍候..."); return; }
    
    modeToggleLock = true;
    pauseRealTimeRefresh();
    
    try {
        const wantOn = !state.turboOn;
        
        if (wantOn && state.naturalWindOn) {
            const nwOk = await safeWrite('NATURE_WIND', '00');
            if (nwOk) {
                state.naturalWindOn = false;
                elements.nwBtn.textContent = '自然风 OFF';
                elements.nwBtn.className = 'btn btn-off';
                await new Promise(r => setTimeout(r, 100));
            } else {
                utils.alert("关闭自然风失败，无法开启 Turbo");
                return;
            }
        }
        
        const ok = await safeWrite('TURBO_MODE', wantOn ? '01' : '00');
        if (ok) {
            state.turboOn = wantOn;
            if (wantOn) {
                elements.turboBtn.textContent = 'Turbo ON';
                elements.turboBtn.className = 'btn btn-turbo-on turbo-btn';
                utils.showToast("Turbo 模式已开启");
            } else {
                elements.turboBtn.textContent = 'Turbo OFF';
                elements.turboBtn.className = 'btn btn-off turbo-btn';
                utils.showToast("Turbo 模式已退出");
            }
        } else {
            utils.alert("Turbo 切换失败");
        }
    } finally {
        modeToggleLock = false;
        setTimeout(() => resumeRealTimeRefresh(), 300);
    }
}

async function toggleNW() {
    if (!state.device?.gatt.connected) { utils.alert("请先连接风扇"); return; }
    if (modeToggleLock) { utils.showToast("请稍候..."); return; }
    
    modeToggleLock = true;
    pauseRealTimeRefresh();
    
    try {
        const wantOn = !state.naturalWindOn;
        
        if (wantOn && state.turboOn) {
            const tbOk = await safeWrite('TURBO_MODE', '00');
            if (tbOk) {
                state.turboOn = false;
                elements.turboBtn.textContent = 'Turbo OFF';
                elements.turboBtn.className = 'btn btn-off turbo-btn';
                await new Promise(r => setTimeout(r, 100));
            } else {
                utils.alert("关闭 Turbo 失败，无法开启自然风");
                return;
            }
        }
        
        const ok = await safeWrite('NATURE_WIND', wantOn ? '01' : '00');
        if (ok) {
            state.naturalWindOn = wantOn;
            elements.nwBtn.textContent = wantOn ? '自然风 ON ' : '自然风 OFF';
            elements.nwBtn.className = wantOn ? 'btn btn-on' : 'btn btn-off';
            if (wantOn) utils.showToast("自然风已开启");
        } else {
            utils.alert("自然风切换失败");
        }
    } finally {
        modeToggleLock = false;
        setTimeout(() => resumeRealTimeRefresh(), 300);
    }
}

async function toggleGearDownMode() {
    if (!state.device?.gatt.connected) { utils.alert("请先连接风扇"); return; }
    const newMode = state.gearDownMode === 0 ? 1 : 0; const hexValue = newMode.toString(16).padStart(2, '0').toUpperCase();
    const success = await safeWrite('GEAR_DOWN_MODE', hexValue);
    if (success) { state.gearDownMode = newMode; if (newMode === 0) { elements.gearDownModeBtn.textContent = '减档: 逐级减档'; elements.gearDownModeBtn.className = 'btn btn-secondary gear-down-mode-btn'; utils.showToast("已设置为逐级减档"); } else if (newMode === 1) { elements.gearDownModeBtn.textContent = '减档: 直接回0'; elements.gearDownModeBtn.className = 'btn btn-warning gear-down-mode-btn'; utils.showToast("已设置为直接回0"); } }
}

function onDisconnected() { updateConnectionStatus(false); cleanupResources(); stopRealTimeRefresh(); }
async function disconnect() { if (state.device?.gatt.connected) state.device.gatt.disconnect(); else await forceDisconnect(); }
async function forceDisconnect() { if (state.device) { try { if (state.device.gatt.connected) state.device.gatt.disconnect(); if (state.disconnectListener) state.device.removeEventListener('gattserverdisconnected', state.disconnectListener); } catch(e) {} } cleanupResources(); stopRealTimeRefresh(); }

function cleanupResources() {
    if (state.device && state.disconnectListener) state.device.removeEventListener('gattserverdisconnected', state.disconnectListener);
    state.device = null; state.mainService = null; state.ffd0Service = null; state.ffc0Service = null; state.natureWindService = null; state.dfuService = null; state.dfuWriteChar = null; state.dfuNotifyChar = null;
    state.cachedChars.clear(); state.isPowerSending = false; state.pendingPowerHex = null; state.isCurveOperationInProgress = false; state.isBatteryOperationInProgress = false;
    state.currentPower = '00'; state.naturalWindOn = false; state.turboOn = false; state.currentTurboTime = 199; state.currentLightBrightness = null; state.bleSnEnabled = null;
    state.isReadingDeviceInfo = false; modeToggleLock = false; realTimeRefreshPaused = false; realTimeRefreshForcePaused = false; stopContinuousAdjust();
    elements.serialNumberValue.textContent = '--'; elements.firmwareVersionValue.textContent = '--'; elements.refreshDeviceInfoBtn.disabled = false; elements.refreshDeviceInfoBtn.textContent = '🔄 获取信息';
    elements.turboBtn.textContent = 'Turbo OFF'; elements.turboBtn.className = 'btn btn-off turbo-btn'; elements.nwBtn.textContent = '自然风 OFF'; elements.nwBtn.className = 'btn btn-off';
    elements.lightStatusDisplay.textContent = '当前: 未设置'; document.querySelectorAll('.light-btn').forEach(btn => btn.classList.remove('btn-on'));
    elements.turboTimeDisplay.textContent = '当前: 未读取'; elements.turboTimeInput.value = 199; elements.powerValue.textContent = '-- W'; elements.motorPowerValue.textContent = '-- W';
    elements.motorCurrentValue.textContent = '-- mA'; elements.motorVoltageValue.textContent = '-- V'; elements.batteryCurrentValue.textContent = '-- mA'; elements.batteryVoltageValue.textContent = '-- V';
    elements.vbusCurrentValue.textContent = '-- mA'; elements.vbusVoltageValue.textContent = '-- V'; elements.chargeStatusValue.textContent = '--'; elements.fanSpeedValue.textContent = '-- %';
    state.lastBatteryVoltage = 0; state.lastBatteryCurrent = 0; state.lastMotorCurrent = 0; state.lastMotorVoltage = 0; state.lastVbusVoltage = 0; state.lastVbusCurrent = 0; state.chargeStatus = 0;
    state.currentFanSpeed = CONFIG.DEFAULT_FAN_SPEED; state.isManualInputFocused = false; state.ignoreNextSpeedUpdate = false; state.speedChangeInProgress = false; state.isSleepDelayInputFocused = false; state.sleepDelayWriteInProgress = false;
    elements.remainingTime.textContent = '剩余时间: 未设置'; elements.currentSpeeds.textContent = '当前设定: 未读取'; elements.batteryCapacityDisplay.textContent = '当前容量: 未读取'; elements.autoSleepDisplay.textContent = '当前: 未设置';
    updateBleSnButton();
    updateConnectionStatus(false);
}

function updateConnectionStatus(connected) { elements.status.textContent = connected ? "✅ 已连接" : "❌ 未连接"; elements.status.className = connected ? "status status-connected" : "status status-disconnected"; }

async function setCustomTimer(minutes = null) {
    let mins = minutes !== null ? minutes : parseInt(elements.customMinutes.value);
    if (isNaN(mins) || mins < 1 || mins > 480) { utils.alert("请输入1-480分钟"); return false; }
    const hex = (mins * 60).toString(16).padStart(4, '0').toUpperCase(); if (await safeWrite('TIMER', hex)) { await readRemainingTime(); return true; } return false;
}
async function cancelTimer() { if (await safeWrite('TIMER', '0000')) await readRemainingTime(); }
async function readRemainingTime() { if (!state.mainService) return; try { let char = state.cachedChars.get(CONFIG.CHARACTERISTICS.TIMER) || await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.TIMER); const secs = (await char.readValue()).getUint16(0, false); elements.remainingTime.textContent = `剩余时间: ${Math.floor(secs/60)}分${secs%60}秒`; } catch(e) { elements.remainingTime.textContent = "剩余时间: 读取失败"; } }
async function setPresetTimer(minutes) { await setCustomTimer(minutes); }

function validateSpeed(inp) { const v = parseInt(inp.value); if (isNaN(v) || v < 0 || v > 100) { inp.style.borderColor = '#ff6b6b'; return false; } inp.style.borderColor = '#e9ecef'; return true; }

async function setCustomSpeeds() {
    const speeds = elements.speedInputs.map(inp => parseInt(inp.value));
    for (let i = 0; i < speeds.length; i++) { if (!validateSpeed(elements.speedInputs[i])) { utils.alert(`档位${i+1}的风速必须在0-100之间`); elements.speedInputs[i].focus(); return; } }
    const hex = speeds.map(s => s.toString(16).padStart(2, '0').toUpperCase()).join('');
    if (await safeWrite('SPEED_CALIB', hex)) { await readCurrentSpeedsWithRetry(); utils.showToast('档位风速已应用'); }
}

async function resetToDefault() {
    elements.speedInputs.forEach((inp, i) => { inp.value = CONFIG.DEFAULT_SPEEDS[i]; inp.style.borderColor = '#e9ecef'; });
    await setCustomSpeeds(); utils.showToast('已恢复默认档位风速');
}

async function setNeverSleep() {
    if (!state.device?.gatt.connected) { utils.alert("请先连接风扇"); return; }
    state.sleepDelayWriteInProgress = true; utils.showToast("正在设置永不休眠...");
    let success = false, retryCount = 0;
    while (retryCount < 3 && !success) {
        try {
            await safeWrite('SHUTDOWN_DELAY', '0000'); await new Promise(r => setTimeout(r, 300));
            let sleepChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY); if (!sleepChar) { sleepChar = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY); state.cachedChars.set(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY, sleepChar); }
            if ((await sleepChar.readValue()).getUint16(0, false) === 0) { success = true; state.autoSleepDelay = 0; elements.autoSleepDisplay.textContent = "当前: 永不关闭"; elements.sleepDelayInput.value = 10; utils.showToast("已设置永不自动休眠"); await refreshSleepDelayImmediately(); } else { retryCount++; if (retryCount < 3) await new Promise(r => setTimeout(r, 500)); }
        } catch(e) { retryCount++; if (retryCount >= 3) utils.alert("设置永不休眠失败，请重试"); else await new Promise(r => setTimeout(r, 500)); }
    }
    state.sleepDelayWriteInProgress = false; resumeRealTimeRefresh();
}

async function applySleepDelay() {
    if (!state.device?.gatt.connected) { utils.alert("请先连接风扇"); return; }
    let delay = parseInt(elements.sleepDelayInput.value); if (isNaN(delay) || delay < 1 || delay > 65535) { utils.alert("请输入1-65535之间的数值！"); return; }
    if (delay >= 1 && delay <= 9) { utils.showToast(`输入值${delay}将自动修正为10秒`); delay = 10; elements.sleepDelayInput.value = 10; }
    const hex = delay.toString(16).padStart(4, '0').toUpperCase(); state.sleepDelayWriteInProgress = true; utils.showToast(`正在设置${delay}秒后自动休眠...`);
    let success = false, retryCount = 0;
    while (retryCount < 3 && !success) {
        try {
            await safeWrite('SHUTDOWN_DELAY', hex); await new Promise(r => setTimeout(r, 300));
            let sleepChar = state.cachedChars.get(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY); if (!sleepChar) { sleepChar = await state.mainService.getCharacteristic(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY); state.cachedChars.set(CONFIG.CHARACTERISTICS.SHUTDOWN_DELAY, sleepChar); }
            if ((await sleepChar.readValue()).getUint16(0, false) === delay) { success = true; state.autoSleepDelay = delay; elements.autoSleepDisplay.textContent = `当前: ${delay}秒后自动关闭`; utils.showToast(`已设置${delay}秒后自动休眠`); await refreshSleepDelayImmediately(); } else { retryCount++; if (retryCount < 3) await new Promise(r => setTimeout(r, 500)); }
        } catch(e) { retryCount++; if (retryCount >= 3) utils.alert(`设置失败，请重试`); else await new Promise(r => setTimeout(r, 500)); }
    }
    state.sleepDelayWriteInProgress = false; resumeRealTimeRefresh();
}

function initCurve() { let arr = CONFIG.DEFAULT_NATURE_WIND_CURVE_DECIMAL.split(/\s+/).map(Number); if (arr.length !== 128) arr = Array(128).fill(50); if (arr.length !== 128) throw new Error('曲线数据不完整'); state.curveData = arr; elements.curveData.value = state.curveData.join(' '); drawCurve(); setupCanvasEvents(); }
function updateCurveFromText() { let vals = elements.curveData.value.trim().split(/\s+/).map(v => parseInt(v)).filter(v => !isNaN(v)); if (vals.length !== 128) { utils.alert(`数据点数量${vals.length}，应为128`); return; } state.curveData = vals; drawCurve(); }
function drawCurve() {
    const canvas = elements.curveCanvas, ctx = canvas.getContext('2d'), w = canvas.parentElement.clientWidth - 24;
    const dpr = window.devicePixelRatio || 1; canvas.width = w * dpr; canvas.height = 120 * dpr; canvas.style.width = w + 'px'; canvas.style.height = '120px'; ctx.scale(dpr, dpr);
    const h = 120, pad = 20, cw = w - 2*pad, ch = h - 2*pad; ctx.clearRect(0,0,w,h); ctx.fillStyle='#fff'; ctx.fillRect(0,0,w,h);
    const minV = Math.min(...state.curveData), maxV = Math.max(...state.curveData), avgV = Math.round(state.curveData.reduce((a,b)=>a+b)/128);
    elements.minValue.textContent = minV; elements.maxValue.textContent = maxV; elements.avgValue.textContent = avgV;
    ctx.strokeStyle='#f0f0f0'; ctx.lineWidth=1; for(let i=0;i<=5;i++){ctx.beginPath();ctx.moveTo(pad,pad+ch*i/5);ctx.lineTo(w-pad,pad+ch*i/5);ctx.stroke();} for(let i=0;i<=8;i++){ctx.beginPath();ctx.moveTo(pad+cw*i/8,pad);ctx.lineTo(pad+cw*i/8,h-pad);ctx.stroke();}
    ctx.strokeStyle='#333';ctx.lineWidth=2;ctx.beginPath();ctx.moveTo(pad,pad);ctx.lineTo(pad,h-pad);ctx.lineTo(w-pad,h-pad);ctx.stroke(); const step=cw/127; ctx.beginPath();ctx.strokeStyle='#667eea';ctx.lineWidth=2;
    for(let i=0;i<128;i++){let x=pad+i*step,y=h-pad-(Math.min(100,Math.max(0,state.curveData[i]))/100*ch);if(i===0)ctx.moveTo(x,y);else ctx.lineTo(x,y);} ctx.stroke();
    for(let i=0;i<128;i++){let x=pad+i*step,y=h-pad-(Math.min(100,Math.max(0,state.curveData[i]))/100*ch);ctx.beginPath();ctx.fillStyle=i===state.selectedPointIndex?'#ff6b6b':'#667eea';ctx.arc(x,y,i===state.selectedPointIndex?8:4,0,Math.PI*2);ctx.fill();}
}
function setupCanvasEvents() {
    const canvas = elements.curveCanvas; const getIdx = (x) => Math.min(127,Math.max(0,Math.round((x-canvas.getBoundingClientRect().left)/(canvas.getBoundingClientRect().width-40)*127)));
    const getVal = (y) => Math.min(100,Math.max(0,Math.round(100-(y-canvas.getBoundingClientRect().top)*(100/(canvas.getBoundingClientRect().height-40)))));
    const startDrag=(cx)=>{state.selectedPointIndex=getIdx(cx);state.isDragging=true;canvas.classList.add('dragging');}; const onMove=(cx,cy)=>{if(!state.isDragging)return;state.curveData[state.selectedPointIndex]=getVal(cy);elements.curveData.value=state.curveData.join(' ');drawCurve();}; const endDrag=()=>{state.isDragging=false;state.selectedPointIndex=-1;canvas.classList.remove('dragging');};
    canvas.addEventListener('mousedown',e=>{e.preventDefault();startDrag(e.clientX);}); window.addEventListener('mousemove',e=>{if(state.isDragging)onMove(e.clientX,e.clientY);}); window.addEventListener('mouseup',endDrag);
    canvas.addEventListener('touchstart',e=>{e.preventDefault();startDrag(e.touches[0].clientX);}); canvas.addEventListener('touchmove',e=>{e.preventDefault();if(state.isDragging)onMove(e.touches[0].clientX,e.touches[0].clientY);}); canvas.addEventListener('touchend',endDrag);
}

async function withCurveOperation(operation, errorMsg) {
    if (state.isCurveOperationInProgress) { utils.showToast("请稍后"); return false; }
    if (!state.device?.gatt.connected) { utils.alert("请先连接"); return false; }
    if (!state.natureWindService) { utils.alert("自然风服务不可用"); return false; }
    state.isCurveOperationInProgress = true; pauseRealTimeRefresh();
    try { return await operation(); } catch(e) { utils.alert(errorMsg); return false; } finally { state.isCurveOperationInProgress = false; resumeRealTimeRefresh(); }
}

async function applyEditedCurve() {
    for (let i = 0; i < state.curveData.length; i++) {
        let val = state.curveData[i];
        if (val < 0 || val > 100) {
            utils.alert(`第${i+1}个数据点 ${val} 超出自然风范围（0-100），请修改后重试。`);
            const textarea = elements.curveData;
            const values = state.curveData.map(v => v.toString());
            let startPos = 0;
            for (let j = 0; j < i; j++) { startPos += values[j].length + 1; }
            const endPos = startPos + values[i].length;
            textarea.focus();
            textarea.setSelectionRange(startPos, endPos);
            return;
        }
    }
    
    return await withCurveOperation(async () => {
        let cc = state.cachedChars.get(CONFIG.CHARACTERISTICS.NATURE_WIND_CURVE);
        if (!cc && state.natureWindService) { cc = await state.natureWindService.getCharacteristic(CONFIG.CHARACTERISTICS.NATURE_WIND_CURVE); state.cachedChars.set(CONFIG.CHARACTERISTICS.NATURE_WIND_CURVE, cc); }
        const data = hexStringToBytes(utils.decimalArrayToHexString(state.curveData));
        if (cc.properties.writeWithoutResponse) await cc.writeValueWithoutResponse(data); else await cc.writeValue(data);
        utils.showToast("曲线已保存" + (state.naturalWindOn ? "" : "，开启自然风后生效"));
    }, "保存曲线失败");
}

async function restoreDefaultCurve() {
    if (!state.device?.gatt.connected) { utils.alert("请先连接"); return; }
    if (!state.natureWindService) { utils.alert("自然风服务不可用"); return; }
    try {
        state.curveData = CONFIG.DEFAULT_NATURE_WIND_CURVE_DECIMAL.split(/\s+/).map(Number);
        elements.curveData.value = state.curveData.join(' '); drawCurve();
        let cc = state.cachedChars.get(CONFIG.CHARACTERISTICS.NATURE_WIND_CURVE);
        if (!cc) { cc = await state.natureWindService.getCharacteristic(CONFIG.CHARACTERISTICS.NATURE_WIND_CURVE); state.cachedChars.set(CONFIG.CHARACTERISTICS.NATURE_WIND_CURVE, cc); }
        if (!cc) { utils.alert("无法获取曲线特征"); return; }
        const data = hexStringToBytes(utils.decimalArrayToHexString(state.curveData));
        let success = false;
        for (let attempt = 0; attempt < 2; attempt++) {
            try {
                if (cc.properties.writeWithoutResponse) await cc.writeValueWithoutResponse(data); else await cc.writeValue(data);
                success = true; break;
            } catch(e) { if (attempt === 0) await new Promise(r => setTimeout(r, 200)); else utils.alert("写入失败: " + e.message); }
        }
        if (success) utils.showToast("默认曲线已还原" + (state.naturalWindOn ? "" : "，开启自然风后生效"));
    } catch(e) { utils.alert("还原默认曲线失败: " + e.message); }
}

async function readNatureWindCurve() {
    return await withCurveOperation(async () => {
        let cc = state.cachedChars.get(CONFIG.CHARACTERISTICS.NATURE_WIND_CURVE); if (!cc && state.natureWindService) { cc = await state.natureWindService.getCharacteristic(CONFIG.CHARACTERISTICS.NATURE_WIND_CURVE); state.cachedChars.set(CONFIG.CHARACTERISTICS.NATURE_WIND_CURVE, cc); }
        const val = await cc.readValue(); const arr = []; for (let i = 0; i < 128 && i < val.byteLength; i++) arr.push(val.getUint8(i));
        if (arr.length !== 128) throw new Error('曲线数据不完整'); state.curveData = arr; elements.curveData.value = state.curveData.join(' '); drawCurve(); utils.showToast("曲线读取成功");
    }, "读取曲线失败");
}

function generateRandomCurve() {
    const s=elements.smoothMode.checked,q=elements.quietMode.checked,t=elements.strongMode.checked;
    let mn=0,mx=100,bv=50,vr=50; if(q){mn=0;mx=30;bv=15;vr=15;} if(t){mn=70;mx=100;bv=85;vr=20;}
    const nd=[]; for(let i=0;i<128;i++){let r=bv+(Math.random()-0.5)*vr;if(Math.random()<0.1)r=mn+Math.random()*(mx-mn);nd.push(Math.min(mx,Math.max(mn,Math.round(r))));}
    if(s)for(let i=1;i<127;i++)nd[i]=Math.round((nd[i-1]+nd[i]+nd[i+1])/3);
    state.curveData=nd; elements.curveData.value=nd.join(' '); drawCurve(); utils.showToast("随机曲线已生成");
}

function calculateEnergy() { const cap=parseFloat(elements.batteryCapacityInput.value)||0,volt=parseFloat(elements.batteryVoltageInput.value)||0; if(cap>0&&volt>0){const wh=(cap*volt/1000).toFixed(2),mwh=Math.round(cap*volt);elements.calculatedEnergy.textContent=wh;elements.calculatedEnergyMWh.textContent=mwh;return mwh;} elements.calculatedEnergy.textContent='--';elements.calculatedEnergyMWh.textContent='--';return 0; }
async function setBatteryCapacity() { if (!state.device?.gatt.connected) { utils.alert("请先连接"); return; } const cap=parseFloat(elements.batteryCapacityInput.value),volt=parseFloat(elements.batteryVoltageInput.value); if(isNaN(cap)||cap<100||cap>50000){utils.alert("容量100-50000");return;} if(isNaN(volt)||volt<3||volt>30){utils.alert("电压3-30");return;} const mwh=Math.round(cap*volt),cmd=`BAT_CAP=${mwh},`; if(await writeAsciiCommand('BATTERY_INFO',cmd)){utils.showToast(`容量已设置 ${mwh}mWh`);state.batteryCapacity=mwh;} }
async function readBatteryCapacity() { if (state.device?.gatt.connected) await refreshRealTimeStatus(); }

function bindEvents() {
    document.getElementById('connectBtn').onclick = connect; document.getElementById('disconnectBtn').onclick = disconnect; document.getElementById('refreshDeviceInfoBtn').onclick = readDeviceInfo;
    document.querySelectorAll('.speed-controls .button-row button[data-power]').forEach(btn => { btn.onclick = () => setPower(btn.dataset.power); });
    elements.turboBtn.onclick = toggleTurbo; elements.nwBtn.onclick = toggleNW; elements.gearDownModeBtn.onclick = toggleGearDownMode;
    elements.bleSnToggleBtn.onclick = toggleBleSn;
    document.querySelectorAll('.light-btn').forEach(btn => { btn.onclick = () => setLightBrightness(btn.dataset.brightness); });
    document.getElementById('applyTurboTimeBtn').onclick = () => setTurboTime(elements.turboTimeInput.value); document.getElementById('resetTurboTimeBtn').onclick = () => setTurboTime(0);
    document.querySelectorAll('.preset-turbo-btn').forEach(btn => { btn.onclick = () => { elements.turboTimeInput.value = btn.dataset.turboTime; setTurboTime(btn.dataset.turboTime); }; });
    elements.turboTimeInput.addEventListener('keypress', e => { if (e.key === 'Enter') setTurboTime(elements.turboTimeInput.value); });
    elements.speedSlider.oninput = e => onSpeedSliderChange(e.target.value);
    elements.speedSlider.onmousedown = elements.speedSlider.ontouchstart = () => { state.isSliderBeingDragged = true; pauseRealTimeRefreshCompletely(); };
    elements.speedSlider.onmouseup = elements.speedSlider.ontouchend = elements.speedSlider.onmouseleave = () => { if (state.isSliderBeingDragged) { sendSpeedImmediately(state.currentFanSpeed); state.isSliderBeingDragged = false; setTimeout(() => { state.ignoreNextSpeedUpdate = true; resumeRealTimeRefresh(); }, 300); } };
    const setupSpeedBtn = (btn, inc) => { let pt=null; const sp=e=>{e.preventDefault();pauseRealTimeRefreshCompletely();adjustSpeed(inc?1:-1);pt=setTimeout(()=>{state.speedAdjustInterval=setInterval(()=>adjustSpeed(inc?1:-1),80);},150);}; const ep=e=>{e.preventDefault();if(pt){clearTimeout(pt);pt=null;}if(state.speedAdjustInterval){clearInterval(state.speedAdjustInterval);state.speedAdjustInterval=null;}setTimeout(()=>{state.ignoreNextSpeedUpdate=true;resumeRealTimeRefresh();},300);}; btn.addEventListener('mousedown',sp,{passive:false});btn.addEventListener('mouseup',ep,{passive:false});btn.addEventListener('mouseleave',ep,{passive:false}); btn.addEventListener('touchstart',sp,{passive:false});btn.addEventListener('touchend',ep,{passive:false});btn.addEventListener('touchcancel',ep,{passive:false}); };
    setupSpeedBtn(elements.decreaseSpeedBtn, false); setupSpeedBtn(elements.increaseSpeedBtn, true);
    elements.manualSpeedInput.onfocus = () => { state.isManualInputFocused = true; }; elements.manualSpeedInput.onblur = () => { state.isManualInputFocused = false; validateAndApplySpeedFromInput(); };
    document.getElementById('setTimerBtn').onclick = () => setCustomTimer(); document.getElementById('cancelTimerBtn').onclick = cancelTimer; document.getElementById('readTimerBtn').onclick = readRemainingTime;
    document.getElementById('preset1h').onclick = () => setPresetTimer(60); document.getElementById('preset4h').onclick = () => setPresetTimer(240);
    document.getElementById('setSpeedsBtn').onclick = setCustomSpeeds; document.getElementById('resetSpeedsBtn').onclick = resetToDefault; document.getElementById('readSpeedsBtn').onclick = readCurrentSpeedsWithRetry;
    document.getElementById('randomCurveBtn').onclick = generateRandomCurve; document.getElementById('applyCurveBtn').onclick = applyEditedCurve; document.getElementById('restoreCurveBtn').onclick = restoreDefaultCurve; document.getElementById('readCurveBtn').onclick = readNatureWindCurve;
    elements.neverSleepBtn.onclick = setNeverSleep; elements.applySleepDelayBtn.onclick = applySleepDelay;
    elements.sleepDelayInput.addEventListener('focus', () => { state.isSleepDelayInputFocused = true; pauseRealTimeRefresh(); });
    elements.sleepDelayInput.addEventListener('blur', () => { state.isSleepDelayInputFocused = false; setTimeout(() => { if (!state.sleepDelayWriteInProgress) resumeRealTimeRefresh(); }, 500); });
    elements.sleepDelayInput.addEventListener('keypress', e => { if (e.key === 'Enter') applySleepDelay(); });
    elements.setBatteryCapacityBtn.onclick = setBatteryCapacity; elements.readBatteryCapacityBtn.onclick = readBatteryCapacity;
    elements.batteryCapacityInput.oninput = calculateEnergy; elements.batteryVoltageInput.oninput = calculateEnergy;
    elements.curveData.oninput = updateCurveFromText;
    
    elements.curveData.onclick = (e) => {
        const ta = e.target;
        const pos = ta.selectionStart;
        let start = pos, end = pos;
        while (start > 0 && ta.value[start - 1] !== ' ') start--;
        while (end < ta.value.length && ta.value[end] !== ' ') end++;
        ta.setSelectionRange(start, end);
    };
    
    elements.curveData.onkeydown = (e) => {
        if (e.key === ' ' || e.key === 'Enter') {
            e.preventDefault();
            const ta = e.target;
            const end = ta.selectionEnd;
            let next = ta.value.indexOf(' ', end);
            if (next === -1) next = ta.value.length;
            else next++;
            let s = next;
            while (s > 0 && ta.value[s - 1] !== ' ') s--;
            let eIdx = next;
            while (eIdx < ta.value.length && ta.value[eIdx] !== ' ') eIdx++;
            ta.setSelectionRange(s, eIdx);
            updateCurveFromText();
        }
    };
    
    elements.manualSpeedInput.addEventListener('blur', () => { let v = parseInt(elements.manualSpeedInput.value); if(isNaN(v)) v=state.currentFanSpeed; v=Math.min(100,Math.max(0,v)); elements.manualSpeedInput.value=v; if(v!==state.currentFanSpeed){state.currentFanSpeed=v;elements.speedSlider.value=v;elements.currentSpeedValue.textContent=`${v}%`;} });
    elements.speedInputs.forEach(inp=>{inp.addEventListener('blur',()=>validateSpeed(inp));});
}

window.addEventListener('load', calculateEnergy); window.addEventListener('resize', drawCurve);
initCurve(); bindEvents();

// C3 transport. The browser never connects to the fan directly.
const bridgeKey = '__W96P_CSRF__';
async function bridgeJob(op, uuid = '', hex = '') {
    const response = await fetch('/api/job', {method:'POST', headers:{'Content-Type':'application/json','X-W96P-Key':bridgeKey}, body:JSON.stringify({op,uuid,hex})});
    const ticket = await response.json();
    if (!response.ok) throw new Error(ticket.error || 'C3 拒绝了操作');
    for (let n=0;n<120;n++) {
        await new Promise(r=>setTimeout(r,250));
        const r=await fetch('/api/job?id='+ticket.id, {cache:'no-store'});
        const result=await r.json();
        if (!r.ok) throw new Error(result.error || '任务已失效');
        if (!result.done) continue;
        if (!result.ok) throw new Error(result.error || '蓝牙操作失败');
        return result;
    }
    throw new Error('C3 操作超时，请刷新状态后再试');
}
function proxyCharacteristic(uuid) {
    return {properties:{writeWithoutResponse:false},
        async readValue() {const r=await bridgeJob('read',uuid);const b=hexStringToBytes(r.hex);return new DataView(b.buffer);},
        async writeValue(bytes) {await bridgeJob('write',uuid,Array.from(bytes,b=>b.toString(16).padStart(2,'0')).join(''));},
        async writeValueWithoutResponse(bytes) {await this.writeValue(bytes);}
    };
}
const proxyService={async getCharacteristic(uuid){return proxyCharacteristic(uuid);}};
async function connect() {
    if (state.isConnecting) return;
    state.isConnecting=true;
    try {
        await bridgeJob('connect');
        state.device={gatt:{connected:true,async disconnect(){try{await bridgeJob('disconnect');onDisconnected();}catch(e){utils.alert(e.message);}}},removeEventListener(){}};
        state.mainService=state.ffd0Service=state.ffc0Service=state.natureWindService=proxyService;
        state.dfuService={};
        updateConnectionStatus(true);
        await Promise.all([readRemainingTime(),readCurrentSpeedsWithRetry(),readNatureWindStatusWithRetry(),readTurboTime(),readLightStatus(),readAutoSleepSettingWithRetry(),readGearDownModeWithRetry(),readBleSnStatus()]);
        startRealTimeRefresh();
    } catch(e) {utils.alert(e.message);updateConnectionStatus(false);}
    finally {state.isConnecting=false;}
}
async function readDeviceInfo() {
    if (state.isReadingDeviceInfo) return;
    state.isReadingDeviceInfo=true;elements.refreshDeviceInfoBtn.disabled=true;
    try {const r=await bridgeJob('info');elements.serialNumberValue.textContent=r.sn||'未读取';elements.firmwareVersionValue.textContent=r.version||'未读取';}
    catch(e){utils.alert(e.message);}
    finally {state.isReadingDeviceInfo=false;elements.refreshDeviceInfoBtn.disabled=false;}
}
async function saveCloud(e) {
    e.preventDefault();const uid=document.getElementById('bemfaUid').value.trim();
    const r=await fetch('/api/cloud',{method:'POST',headers:{'Content-Type':'application/json','X-W96P-Key':bridgeKey},body:JSON.stringify({uid})});
    const j=await r.json();document.getElementById('cloudMessage').textContent=r.ok?'已保存；等待云端连接':j.error;
    if(r.ok)document.getElementById('bemfaUid').value='';
}
async function saveWifi(e) {
    e.preventDefault();const ssid=document.getElementById('wifiSsid').value,password=document.getElementById('wifiPass').value;
    const r=await fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json','X-W96P-Key':bridgeKey},body:JSON.stringify({ssid,password})});
    const j=await r.json();document.getElementById('wifiMessage').textContent=r.ok?'已提交 Wi-Fi 连接；连上后请从路由器查看 C3 的新地址。':j.error;
    if(r.ok)document.getElementById('wifiPass').value='';
}
setInterval(async()=>{try{const s=await(await fetch('/api/status',{cache:'no-store'})).json();document.getElementById('bridgeStatus').textContent='Wi-Fi：'+s.ip+' · 蓝牙：'+(s.ble?'已连接':'未连接')+' · 巴法：'+(s.cloud?'已连接':s.configured?'连接中':'未配置');if(!s.ble&&state.device?.gatt.connected)onDisconnected();}catch{}},3000);

</script>
</body>
</html>)W96PAGE";
