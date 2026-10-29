"""Open light once and trace whether the hardware or a command closes it."""
import json,time
from pathlib import Path
import serial
root=Path(__file__).resolve().parents[1]
s=serial.Serial('COM12',115200,timeout=.05)
events=[];rows=[]
def line():
    v=s.readline().decode(errors='replace').strip()
    if v.startswith(('LIGHT_','CLOUD_','SPEED_')):
        events.append({'elapsed':round(time.monotonic()-begin,3),'line':v});print(v,flush=True)
    return v
def job(op,**args):
    data=('@'+json.dumps({'op':op,**args})+'\n').encode();s.write(data);s.flush();ticket=None;end=time.monotonic()+25
    while time.monotonic()<end:
        v=line()
        if not v.startswith('@'):continue
        try:r=json.loads(v[1:])
        except ValueError:continue
        if r.get('accepted') is False:raise RuntimeError('rejected')
        if r.get('accepted'):ticket=r['id']
        if r.get('done') and r.get('id')==ticket:
            if not r['ok']:raise RuntimeError(r.get('error'))
            return r
    raise TimeoutError(op)
def read(u):return int.from_bytes(bytes.fromhex(job('read',uuid=u)['hex']),'big')
def pump(seconds):
    end=time.monotonic()+seconds
    while time.monotonic()<end:line()
begin=time.monotonic();original=None
try:
    job('connect');original=read('fffa');job('light',value=4);start=time.monotonic()
    while time.monotonic()-start<45:
        row={'seconds':round(time.monotonic()-start,2),'light':read('fffa'),'power':read('fff1')};rows.append(row);print(json.dumps(row),flush=True);pump(2)
finally:
    try:
        if original is not None:job('light',value=original)
    finally:
        s.close();(root/'dist/light-timeout-observation.json').write_text(json.dumps({'baselineLight':original,'events':events,'samples':rows},ensure_ascii=False,indent=2),encoding='utf-8')
