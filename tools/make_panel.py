"""Preserve the vendor UI; replace only Web Bluetooth transport with C3 HTTP jobs."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
source = root/'tools'/'w96p-vendor.html'
html = source.read_text(encoding='utf-8')

bridge = r'''
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
'''

# Replace function declarations, not their call sites.
def replace_function(text, name, replacement):
    pattern = rf'(?m)^async function {name}\([^\n]*\) \{{'
    match = re.search(pattern, text)
    if not match:
        raise ValueError(f'missing function {name}')
    # All three targeted vendor functions end with a column-zero brace.
    end = re.search(r'(?m)^\}', text[match.end():])
    if end is None:
        raise ValueError(name)
    return text[:match.start()] + replacement + text[match.end()+end.end():]

html = replace_function(html, 'connect', '')
html = replace_function(html, 'readDeviceInfo', '')
# Coalesce rapid input instead of dropping the final requested value over HTTP.
html = replace_function(html, 'sendSpeedImmediately', r'''async function sendSpeedImmediately(speed) {
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
}''')
html = html.replace('</script>', bridge + '\n</script>')
html = html.replace('蓝牙智能控制面板', 'C3 本地控制 · 米家桥接')
cloud = '''<div class="container"><div class="card"><h3>C3 桥接</h3>
<p id="bridgeStatus" role="status">正在读取连接状态…</p>
<details><summary>配置巴法云</summary><p>创建 w96speed002（风速）和 w96light002（实体灯）主题后填写 UID。云端未配置也能使用本地控制。</p>
<form onsubmit="saveCloud(event)"><label for="bemfaUid">巴法私钥 UID</label><input id="bemfaUid" type="password" autocomplete="off" maxlength="32" required><button class="btn btn-primary" type="submit">保存</button><p id="cloudMessage" role="status"></p></form>
<p>UID 仅在本地填写；此页面通过局域网 HTTP 访问，请使用自己的可信网络。</p></details>
<details><summary>设置 Wi-Fi</summary><form onsubmit="saveWifi(event)"><div class="input-group"><label for="wifiSsid">2.4GHz 名称</label><input id="wifiSsid" maxlength="32" required autocomplete="off"></div><div class="input-group"><label for="wifiPass">Wi-Fi 密码</label><input id="wifiPass" type="password" maxlength="63" autocomplete="new-password"></div><button class="btn btn-primary" type="submit">连接 Wi-Fi</button><p id="wifiMessage" role="status"></p></form></details>
<p>首次使用请点「连接风扇」。开关机或断线后，读数以重新读取结果为准。</p></div></div>'''
html = html.replace('<body>', '<body>\n'+cloud)
# Stop a failed prerequisite from falling through into a different mode.
html = html.replace("state.turboOn = false; await safeWrite('TURBO_MODE', '00');", "if (!await safeWrite('TURBO_MODE', '00')) throw new Error('关闭 Turbo 失败'); state.turboOn = false;")
html = html.replace("await safeWrite('NATURE_WIND', '00'); state.naturalWindOn = false;", "if (!await safeWrite('NATURE_WIND', '00')) throw new Error('关闭自然风失败'); state.naturalWindOn = false;")
html = html.replace("await safeWrite('POWER', '01'); state.currentPower = '01';", "if (!await safeWrite('POWER', '01')) throw new Error('开启风扇失败'); state.currentPower = '01';")
html = html.replace("await safeWrite('POWER', hexValue, true); state.currentPower = hexValue;", "if (!await safeWrite('POWER', hexValue, true)) return false; state.currentPower = hexValue;")
html = html.replace("return safeWrite(characteristicKey, hexValue, false); } return false;", "return safeWrite(characteristicKey, hexValue, false); } utils.alert(e.message || '写入未确认'); return false;")
html = html.replace('function setLightBrightness(hexValue) {', 'async function setLightBrightness(hexValue) {')
html = html.replace("    safeWrite('LIGHT_CONTROL', hexValue);", "    if (!await safeWrite('LIGHT_CONTROL', hexValue)) return;")
html = html.replace('function validateAndApplySpeedFromInput() {', 'async function validateAndApplySpeedFromInput() {')
html = html.replace('sendSpeedImmediately(speed); utils.showToast', 'await sendSpeedImmediately(speed); utils.showToast')
html = html.replace("state.curveData = arr; elements.curveData.value", "if (arr.length !== 128) throw new Error('曲线数据不完整'); state.curveData = arr; elements.curveData.value")
html = html.replace("首次使用请点「连接风扇」。", "C3 会自动连接风扇；打开面板后点「连接风扇」加载控制状态。")
html = html.replace('id="sleepDelayInput"', 'id="sleepDelayInput" disabled title="米家常在线模式：蓝牙保持永不关闭"')
html = html.replace('id="applySleepDelayBtn">应用设置', 'id="applySleepDelayBtn" disabled>米家常在线：禁止休眠')
# A full refresh must finish before another interval can enqueue work.
html = html.replace('async function refreshRealTimeStatus() {', 'let bridgeRefreshing=false;\nasync function vendorRefreshRealTimeStatus() {')
html = html.replace('function startRealTimeRefresh() {', 'async function refreshRealTimeStatus(){if(bridgeRefreshing)return;bridgeRefreshing=true;try{await vendorRefreshRealTimeStatus();}finally{bridgeRefreshing=false;}}\nfunction startRealTimeRefresh() {')
# Do not retain the vendor's unsigned-to-signed serial conversion path.
# readDeviceInfo above already uses the C3 uint32 result.
# Keep all assets local; original has only inline CSS, JS and a canvas.
if 'navigator.bluetooth' in html:
    raise ValueError('Web Bluetooth was not fully removed')
(root/'include'/'panel.h').write_text('#pragma once\n#include <Arduino.h>\nstatic const char PANEL_HTML[] PROGMEM = R"W96PAGE('+html+')W96PAGE";\n',encoding='utf-8')
print(f'Generated panel: {len(html.encode())} bytes')
crc = re.search(r'const CRC8_TABLE = (\[[\s\S]*?\]);', source.read_text(encoding='utf-8')).group(1)
values=list(map(int,re.findall(r'\d+',crc)))
# The vendor table is CRC-8/poly 0x3B except for two overwritten entries.
# Correct the two values so arbitrary response keys/CRC paths decode reliably.
assert len(values)==256 and values[120] in (200,102) and values[202] in (200,101)
values[120]=102;values[202]=101
(root/'include'/'vendor_crc.h').write_text('#pragma once\n// CRC-8 0x3B; corrected vendor entries 120 and 202.\nstatic const uint8_t VENDOR_CRC[256]={'+','.join(map(str,values))+'};\n',encoding='utf-8')
ca = (root/'include'/'bemfa_root.pem').read_text()
(root/'include'/'bemfa_ca.h').write_text('#pragma once\nstatic const char BEMFA_ROOT_CA[] PROGMEM = R"CERT('+ca+')CERT";\n',encoding='utf-8')
