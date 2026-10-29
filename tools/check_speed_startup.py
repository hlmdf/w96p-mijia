"""Measure whether FFF3 can stage speed before power-on; restore initial state."""
import json,time
from pathlib import Path
from hardware_check import Bridge
b=Bridge('COM12');events=[]
def snapshot(label):
    p=b.read('fff1')[0];s=b.read('fff3')[0]
    v={'step':label,'power':p,'speed':s};events.append(v);print(json.dumps(v),flush=True);return p,s
original_p,original_s=snapshot('baseline')
try:
    b.job('speed',value=1);snapshot('set 1')
    b.job('speed',value=0);snapshot('off')
    b.write('fff3',bytes([43]));snapshot('write 43 while off')
    b.write('fff1',bytes([1]));snapshot('power on after staging 43')
finally:
    if original_p:
        b.write('fff1',bytes([original_p]));b.write('fff3',bytes([original_s]))
    else:
        b.write('fff3',bytes([original_s]));b.write('fff1',bytes([0]))
    snapshot('restored');b.port.close()
    (Path(__file__).resolve().parents[1]/'dist/speed-startup-check.json').write_text(json.dumps(events,indent=2),encoding='utf-8')
