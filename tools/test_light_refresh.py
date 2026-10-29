"""Temporary repeated-brightness experiment; visual observation is required.
Stop early by creating dist/light-refresh.stop. Always attempt original-level restore.
"""
import json,time
from pathlib import Path
from hardware_check import Bridge
root=Path(__file__).resolve().parents[1]
stop=root/'dist/light-refresh.stop'
if stop.exists():stop.unlink()
b=Bridge('COM12');report={'samples':[]};original=None
try:
    b.job('connect');original=b.read('fffa')[0];report['originalLight']=original
    report['originalPower']=b.read('fff1')[0];report['originalSpeed']=b.read('fff3')[0]
    start=time.monotonic();print('LIGHT REFRESH ACTIVE FOR UP TO 120 SECONDS',flush=True)
    while time.monotonic()-start<120 and not stop.exists():
        b.job('light',value=4)
        row={'seconds':round(time.monotonic()-start,2),'level':b.read('fffa')[0]};report['samples'].append(row);print(json.dumps(row),flush=True)
        end=time.monotonic()+3
        while time.monotonic()<end and not stop.exists():time.sleep(.1)
finally:
    try:
        if original is not None:b.job('light',value=original)
        report['finalPower']=b.read('fff1')[0];report['finalSpeed']=b.read('fff3')[0]
        print('ORIGINAL LIGHT RESTORED; MOTOR UNCHANGED '+str(report['finalPower'])+'/'+str(report['finalSpeed']),flush=True)
    finally:
        b.port.close();(root/'dist/light-refresh-test.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
