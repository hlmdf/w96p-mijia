"""Verify manual disconnect holds, then a full C3 reset auto-connects without a connect command."""
import json,subprocess,sys,time,urllib.request
from pathlib import Path
from hardware_check import Bridge
root=Path(__file__).resolve().parents[1]
opener=urllib.request.build_opener(urllib.request.ProxyHandler({}))
def status():
    with opener.open('http://192.168.0.251/api/status',timeout=3) as r:return json.load(r)
report={'manual':[],'boot':[]}
b=Bridge('COM12')
try:
    b.job('disconnect');time.sleep(2)
    for _ in range(6):
        state=status();report['manual'].append(state)
        if state['ble']:raise AssertionError('Manual disconnect was overridden')
        time.sleep(2)
    print('MANUAL DISCONNECT HOLDS: PASS',flush=True)
finally:b.port.close()
# esptool enters ROM and then hard-resets the C3; it does not touch the fan.
p=subprocess.run([sys.executable,'-m','esptool','--chip','esp32c3','--port','COM12','chip-id'],capture_output=True,text=True,timeout=25)
if p.returncode:raise RuntimeError('C3 reset failed')
print('C3 HARD RESET DONE; NO BLE CONNECT COMMAND WILL BE SENT',flush=True)
deadline=time.monotonic()+45;last=None;success=False
while time.monotonic()<deadline:
    try:
        s=status();report['boot'].append(s)
        key=(s['ble'],s.get('bleKeepAwake'),s['cloud'])
        if key!=last:print('BOOT STATUS '+json.dumps(s),flush=True);last=key
        if all(key):success=True;break
    except Exception:pass
    time.sleep(1)
report['pass']=success
(root/'dist/boot-connection-test.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
if not success:raise AssertionError('Boot did not recover BLE, keep-awake and cloud within 45 seconds')
print('BOOT AUTO-CONNECT + CLOUD + NEVER-SLEEP: PASS',flush=True)
