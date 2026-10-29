import json,time
from pathlib import Path
from hardware_check import Bridge
root=Path(__file__).resolve().parents[1]
b=Bridge('COM12',paced=False)
report={'checks':[],'restored':{}}
def check(name,fn):
    try:
        value=fn()
        if value is False:raise AssertionError('Readback does not match expected value')
        report['checks'].append({'name':name,'ok':True,'result':value});print(name+': PASS '+str(value),flush=True)
    except Exception as e:
        report['checks'].append({'name':name,'ok':False,'error':str(e)});print(name+': FAIL '+str(e),flush=True)
try:
    check('connect',lambda:b.job('connect')['ok'])
    curve=b.read('ffe3')
    check('full USB curve request',lambda:b.write('ffe3',curve)['ok'])
    check('curve exact readback',lambda:b.read('ffe3')==curve)
    check('device info with acknowledged subscription',lambda:{k:v for k,v in b.job('info').items() if k in ('sn','version')})
    power=b.read('fff1');speed=b.read('fff3');gears=b.read('fff7')
    try:
        check('fan off',lambda:(b.job('speed',value=0),b.read('fff1')[0]==0)[1])
        check('fan on and exact percent',lambda:(b.job('speed',value=max(1,speed[0])),b.read('fff1')[0]>0 and b.read('fff3')==speed)[1])
        changed=bytes([(curve[0]+1)%101])+curve[1:]
        check('curve actual one-point change',lambda:(b.write('ffe3',changed),b.read('ffe3')==changed)[1])
        changed_gears=gears[:1]+bytes([(gears[1]+1)%101])+gears[2:]
        check('gear actual second-level change',lambda:(b.write('fff7',changed_gears),b.read('fff7')==changed_gears)[1])
        gear_mode=b.read('fff6')
        check('gear-down mode change',lambda:(b.write('fff6',bytes([1-gear_mode[0]])),b.read('fff6')[0]==1-gear_mode[0])[1])
    finally:
        b.write('fff6',bytes.fromhex(json.loads((root/'dist/hardware-check.json').read_text(encoding='utf-8'))['read']['fff6']))
        b.write('ffe3',curve);b.write('fff7',gears);b.write('fff1',power);b.write('fff3',speed)
    baseline=json.loads((root/'dist/hardware-check.json').read_text(encoding='utf-8'))['read']
    for u in ['fff1','fff3','fffa','fff2','fff4','fff5','fff6','fff7','fff8','fffb','ffe3']:
        value=b.read(u).hex();matches=value==baseline[u];report['restored'][u]={'matches':matches,'hex':value};print('RESTORED '+u+': '+str(matches),flush=True)
finally:
    b.port.close();(root/'dist/hardware-followup.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
