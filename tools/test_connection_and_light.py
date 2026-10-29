"""Hardware checks for keep-awake, coupling, independent lighting and reconnect."""
import json,time,urllib.request
from pathlib import Path
from hardware_check import Bridge
root=Path(__file__).resolve().parents[1]
report={'baseline':{},'tests':[]}
b=Bridge('COM12')
def value(u):return int.from_bytes(b.read(u),'big')
def check(name,fn):
    try:
        result=fn()
        if result is False:raise AssertionError('readback mismatch')
        report['tests'].append({'name':name,'pass':True,'result':result});print(name+': PASS '+str(result),flush=True);return result
    except Exception as e:
        report['tests'].append({'name':name,'pass':False,'error':str(e)});print(name+': FAIL '+str(e),flush=True);raise
def eventually(fn,timeout=15):
    until=time.monotonic()+timeout;last=''
    while time.monotonic()<until:
        try:
            if fn():return True
        except Exception as e:last=str(e)
        time.sleep(.25)
    raise AssertionError('Condition not reached: '+last)
def status():
    opener=urllib.request.build_opener(urllib.request.ProxyHandler({}))
    with opener.open('http://192.168.0.251/api/status',timeout=4) as r:return json.load(r)
def reject_sleep():
    try:b.write('fff5',bytes([0,60]))
    except RuntimeError:return value('fff5')==0
    return False
baseline=None
try:
    b.job('connect');baseline={u:value(u) for u in ['fff1','fff3','fffa','fff2','fff5']};report['baseline']=baseline
    check('BLE never-sleep confirmed',lambda:value('fff5')==0)
    check('automatic sleep rejected by online policy',reject_sleep)

    b.job('speed',value=21);b.job('light',value=3);b.job('cloudspeed',message='off')
    check('cloud fan off also turns light off',lambda:eventually(lambda:value('fff1')==0 and value('fffa')==0))
    check('BLE still readable after off',lambda:value('fff5')==0)

    b.job('light',value=4);time.sleep(6)
    check('independent light survives while motor off',lambda:value('fff1')==0 and value('fffa')==4)
    b.job('speed',value=21);b.job('light',value=2);b.job('light',value=0)
    check('independent light off leaves motor running',lambda:value('fff1')>0 and value('fff3')==21 and value('fffa')==0)

    b.job('light',value=2);b.write('fff1',bytes([0]))
    check('local power off also turns light off',lambda:value('fff1')==0 and value('fffa')==0 and value('fff5')==0)

    before=status();b.job('linkdrop')
    check('link loss automatically reconnects without a connect command',lambda:eventually(lambda:status()['ble'] and status()['bleKeepAwake'] and status()['bleReconnectAttempts']>before['bleReconnectAttempts'],40))

    if baseline['fff2']==0:
        b.job('speed',value=21);b.job('light',value=2);b.write('fff2',(2).to_bytes(2,'big'))
        check('native timer shutdown also turns light off',lambda:eventually(lambda:value('fff1')==0 and value('fffa')==0,12))
        b.write('fff2',bytes([0,0]))
finally:
    try:
        if baseline:
            b.job('connect')
            b.job('speed',value=baseline['fff3'] if baseline['fff1'] else 0)
            b.job('light',value=baseline['fffa'])
            report['restored']={u:value(u) for u in ['fff1','fff3','fffa','fff5']}
            print('RESTORED '+str(report['restored']),flush=True)
    finally:
        b.port.close();(root/'dist/connection-and-light-test.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
