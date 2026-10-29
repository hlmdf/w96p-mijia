"""Final regression: autonomous hold with power guard and no relight after native off."""
import json,time,urllib.request
from pathlib import Path
from hardware_check import Bridge
root=Path(__file__).resolve().parents[1]
opener=urllib.request.build_opener(urllib.request.ProxyHandler({}))
def status():
    with opener.open('http://192.168.0.251/api/status',timeout=4) as r:return json.load(r)
b=Bridge('COM12');report={'samples':[]};original=None
def read(u):return int.from_bytes(b.read(u),'big')
try:
    b.job('connect');original={'power':read('fff1'),'speed':read('fff3'),'light':read('fffa'),'timer':read('fff2')};report['baseline']=original
    b.job('light',value=4);before=status()['lightRefreshCount'];start=time.monotonic()
    print('FINAL AUTONOMOUS HOLD CHECK: 45 seconds of HTTP-only reads',flush=True)
    while time.monotonic()-start<45:
        row=status();row['elapsed']=round(time.monotonic()-start,1);report['samples'].append(row)
        if len(report['samples'])%5==1:print('HOLD '+str(row['elapsed'])+' refreshes='+str(row['lightRefreshCount']),flush=True)
        time.sleep(2)
    assert status()['lightIntent']==4 and status()['lightRefreshCount']-before>=6
    report['holdPass']=True;print('AUTONOMOUS HOLD PASS',flush=True)
    if original['timer']==0:
        b.job('speed',value=21);b.job('light',value=2);b.write('fff2',bytes([0,2]));end=time.monotonic()+15
        while time.monotonic()<end:
            if read('fff1')==0 and status()['lightIntent']==0:break
            time.sleep(.2)
        assert read('fff1')==0 and read('fffa')==0 and status()['lightIntent']==0
        count=status()['lightRefreshCount'];time.sleep(8)
        assert read('fffa')==0 and status()['lightRefreshCount']==count
        report['noRelightPass']=True;print('NATIVE SHUTDOWN CANCELS HOLD; NO RELIGHT PASS',flush=True)
        b.write('fff2',bytes([0,0]))
finally:
    try:
        if original:
            b.job('speed',value=original['speed'] if original['power'] else 0);b.job('light',value=original['light'])
            report['restored']={'power':read('fff1'),'speed':read('fff3'),'light':read('fffa')};print('RESTORED '+str(report['restored']),flush=True)
    finally:
        b.port.close();(root/'dist/light-hold-guard-test.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
