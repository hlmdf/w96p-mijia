"""Verify autonomous light-off without host light refreshes."""
import json,time
from pathlib import Path
from hardware_check import Bridge

b=Bridge('COM12')
report={'checks':[]}
original=None
def check(name,ok,**details):
    report['checks'].append(dict(name=name,ok=bool(ok),**details))
    print(json.dumps(report['checks'][-1],ensure_ascii=False),flush=True)
    assert ok,name
def wait_off(timeout=23):
    start=time.monotonic()
    while time.monotonic()-start<timeout:
        value=b.read('fffa')[0]
        if value==0:return round(time.monotonic()-start,2)
        time.sleep(1)
    raise AssertionError('Light did not automatically turn off')
try:
    b.job('connect')
    original=(b.read('fff1')[0],b.read('fff3')[0])
    b.job('speed',value=0)
    b.job('light',value=4)
    elapsed=wait_off()
    check('auto-off while motor off',13<=elapsed<23,seconds=elapsed,power=b.read('fff1')[0])
    b.job('speed',value=21)
    b.job('light',value=3)
    time.sleep(9)
    b.job('light',value=2)
    time.sleep(7)
    check('brightness operation restarts timer',b.read('fffa')[0]==2)
    elapsed=wait_off(16)
    check('auto-off while motor running',b.read('fff1')[0]>0 and b.read('fff3')[0]==21,seconds_after_second_check=elapsed)
    check('Bluetooth remains awake',b.read('fff5')==b'\x00\x00')
finally:
    try:
        if original is not None:b.job('speed',value=original[1] if original[0] else 0)
        b.job('light',value=0)
        report['final']={u:b.read(u).hex() for u in ('fff1','fff3','fffa','fff5')}
        print('FINAL '+json.dumps(report['final']),flush=True)
    finally:
        b.port.close()
        (Path(__file__).resolve().parents[1]/'dist'/'light-auto-off-test.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
