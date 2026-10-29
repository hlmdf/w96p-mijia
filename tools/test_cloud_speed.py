"""Exercise the real MQTT callback via USB injection; BLE writes are real.

This verifies callback scheduling + GATT delivery, not speech recognition.
"""
import json,re,time
from pathlib import Path
import serial
root=Path(__file__).resolve().parents[1]
s=serial.Serial('COM12',115200,timeout=.02)
events=[];report={'tests':[]}
def line():
    v=s.readline().decode(errors='replace').strip()
    if v.startswith(('CLOUD_SPEED','SPEED_')):events.append(v);print(v,flush=True)
    return v
def pump(seconds):
    until=time.monotonic()+seconds
    while time.monotonic()<until:line()
def job(op,**args):
    payload=('@'+json.dumps({'op':op,**args})+'\n').encode()
    for i in range(0,len(payload),48):s.write(payload[i:i+48]);time.sleep(.005)
    s.flush();deadline=time.monotonic()+20;ticket=None
    while time.monotonic()<deadline:
        v=line()
        if not v.startswith('@'):continue
        try:r=json.loads(v[1:])
        except ValueError:continue
        if r.get('accepted') is False:raise RuntimeError('request rejected')
        if r.get('accepted'):ticket=r['id']
        if r.get('done') and r.get('id')==ticket:
            if not r['ok']:raise RuntimeError(r.get('error','BLE failed'))
            return r
    raise TimeoutError(op)
def read(u):return bytes.fromhex(job('read',uuid=u)['hex'])
def inject(message):job('cloudspeed',message=message)
def targets(start):return [int(re.search(r'target=(\d+)',e)[1]) for e in events[start:] if e.startswith('SPEED_APPLIED')]
def check(name,condition,details):
    report['tests'].append({'name':name,'pass':bool(condition),'details':details})
    print(name+': '+('PASS' if condition else 'FAIL')+' '+str(details),flush=True)
    if not condition:raise AssertionError(name)
job('connect');original_p=read('fff1')[0];original_s=read('fff3')[0]
restore_speed=original_s
try:
    if not original_p:
        inject('on');pump(1.0);restore_speed=read('fff3')[0]
        inject('off');pump(.1);assert read('fff1')[0]==0
    job('speed',value=1);inject('off');pump(.1);assert read('fff1')[0]==0
    start=len(events);inject('on');pump(.04);inject('on#43');pump(.9)
    speed=read('fff3')[0];applied=targets(start)
    check('remembered 1 / off / on + 43',speed==43 and applied==[43],{'speed':speed,'targets':applied})

    inject('off');pump(.1);assert read('fff1')[0]==0;start=len(events)
    inject('on#10');pump(.25);inject('on');pump(.9)
    speed=read('fff3')[0];applied=targets(start)
    check('percent then delayed companion on',speed==10 and applied==[10],{'speed':speed,'targets':applied})

    job('speed',value=21);inject('off');pump(.1);assert read('fff1')[0]==0;start=len(events)
    inject('on');pump(1.0);speed=read('fff3')[0];applied=targets(start)
    check('standalone on remembers last speed',speed==21 and applied==[21],{'speed':speed,'targets':applied})

    inject('off');pump(.1);assert read('fff1')[0]==0;start=len(events);inject('on');pump(.04);inject('off');pump(.9)
    power=read('fff1')[0];applied=targets(start)
    check('off cancels pending restore',power==0 and applied==[0],{'power':power,'targets':applied})

    inject('on#43');pump(.9);check('percent alone starts at target',read('fff3')[0]==43,{'targets':targets(start)})
finally:
    try:
        if original_p:job('speed',value=original_s)
        else:job('speed',value=restore_speed);job('speed',value=0)
        report['restored']={'power':read('fff1')[0],'speed':read('fff3')[0]}
        print('RESTORED '+str(report['restored']),flush=True)
    finally:
        s.close();report['trace']=events;(root/'dist/cloud-speed-fix-test.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
