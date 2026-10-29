"""Verify autonomous firmware refresh (no host refresh), off cancellation and persistence."""
import json,subprocess,sys,time,urllib.request
from pathlib import Path
from hardware_check import Bridge
root=Path(__file__).resolve().parents[1]
opener=urllib.request.build_opener(urllib.request.ProxyHandler({}))
def status():
    with opener.open('http://192.168.0.251/api/status',timeout=4) as r:return json.load(r)
def wait_for(fn,seconds=30):
    end=time.monotonic()+seconds
    while time.monotonic()<end:
        try:
            if fn():return True
        except Exception:pass
        time.sleep(.5)
    raise AssertionError('Condition timed out')
report={'tests':[],'autonomousSamples':[]}
b=Bridge('COM12');baseline=None
def value(u):return int.from_bytes(b.read(u),'big')
def check(name,ok,detail=None):
    report['tests'].append({'name':name,'pass':bool(ok),'details':detail});print(name+': '+('PASS' if ok else 'FAIL')+' '+str(detail),flush=True)
    if not ok:raise AssertionError(name)
try:
    b.job('connect');baseline={u:value(u) for u in ['fff1','fff3','fffa','fff2']};report['baseline']=baseline
    b.job('light',value=4);initial=status()['lightRefreshCount'];start=time.monotonic();lastPrinted=-10
    print('AUTONOMOUS HOLD: host will only read HTTP status for 45 seconds',flush=True)
    while time.monotonic()-start<45:
        row=status();row['elapsed']=round(time.monotonic()-start,1);report['autonomousSamples'].append(row)
        if row['elapsed']-lastPrinted>=10:print('HOLD '+json.dumps(row),flush=True);lastPrinted=row['elapsed']
        time.sleep(2)
    after=status();check('firmware independently refreshes light',after['lightIntent']==4 and after['lightRefreshCount']-initial>=6,{'refreshes':after['lightRefreshCount']-initial})
    b.job('light',value=0);off=status();time.sleep(8);later=status()
    check('explicit off stops hold and stays off',value('fffa')==0 and later['lightIntent']==0 and later['lightRefreshCount']==off['lightRefreshCount'])
    b.job('speed',value=21);b.job('light',value=3);b.job('speed',value=0);off=status();time.sleep(7);later=status()
    check('fan off cancels hold and shuts light',value('fff1')==0 and value('fffa')==0 and later['lightIntent']==0 and later['lightRefreshCount']==off['lightRefreshCount'])
    b.job('light',value=2);before=status()['lightRefreshCount'];time.sleep(7)
    check('independent light holds while fan off',value('fff1')==0 and value('fffa')==2 and status()['lightRefreshCount']>before)
    if baseline['fff2']==0:
        b.job('speed',value=21);b.job('light',value=2);b.write('fff2',bytes([0,2]));wait_for(lambda:value('fff1')==0 and value('fffa')==0 and status()['lightIntent']==0,15)
        check('native timer cancels hold',True);b.write('fff2',bytes([0,0]))
    b.job('light',value=2);b.port.close();b=None
    p=subprocess.run([sys.executable,'-m','esptool','--chip','esp32c3','--port','COM12','chip-id'],capture_output=True,text=True,timeout=25)
    if p.returncode:raise RuntimeError('Reset failed')
    wait_for(lambda:status()['ble'] and status()['lightIntent']==2 and status()['lightRefreshCount']>=2,40)
    check('light intent persists and refresh resumes after C3 reset',True,status())
finally:
    try:
        if baseline:
            if b is None:b=Bridge('COM12')
            b.job('connect');b.job('speed',value=baseline['fff3'] if baseline['fff1'] else 0);b.job('light',value=baseline['fffa'])
            report['restored']={u:value(u) for u in ['fff1','fff3','fffa']};print('RESTORED '+str(report['restored']),flush=True)
    finally:
        if b:b.port.close()
        (root/'dist/firmware-light-hold-test.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
