"""Operate the user's confirmed fan via the C3's real BLE worker over USB.

Default is read-only. --controls runs bounded reversible checks with restoration.
No Wi-Fi credentials or cloud secrets are read or printed.
"""
import argparse,json,time
from pathlib import Path
import serial

class Bridge:
    def __init__(self,port,paced=True):
        self.port=serial.Serial(port,115200,timeout=.3)
        self.paced=paced
    def job(self,op,**args):
        self.port.reset_input_buffer()
        request=('@'+json.dumps(dict(op=op,**args))+'\n').encode()
        if self.paced:
            for start in range(0,len(request),48):
                self.port.write(request[start:start+48]);time.sleep(.015)
        else:self.port.write(request)
        self.port.flush();deadline=time.monotonic()+35;ticket=None
        while time.monotonic()<deadline:
            line=self.port.readline().decode(errors='replace').strip()
            if not line.startswith('@'):continue
            try:r=json.loads(line[1:])
            except ValueError:continue
            if r.get('accepted') is False:raise RuntimeError('C3 rejected command')
            if r.get('accepted'):ticket=r['id']
            if r.get('done') and r.get('id')==ticket:
                if not r.get('ok'):raise RuntimeError(r.get('error') or 'BLE operation failed')
                return r
        raise TimeoutError(op+' timed out')
    def read(self,u):return bytes.fromhex(self.job('read',uuid=u)['hex'])
    def write(self,u,b):return self.job('write',uuid=u,hex=b.hex())

def main():
    p=argparse.ArgumentParser();p.add_argument('--port',default='COM12');p.add_argument('--controls',action='store_true');a=p.parse_args()
    bridge=Bridge(a.port);report={'timestamp':time.strftime('%Y-%m-%d %H:%M:%S'),'read':{},'checks':[]}
    def check(name,fn):
        try:result=fn();report['checks'].append({'name':name,'ok':True,'result':result});print(name+': PASS '+str(result),flush=True);return result
        except Exception as e:report['checks'].append({'name':name,'ok':False,'error':str(e)});print(name+': FAIL '+str(e),flush=True);return None
    try:
        if check('connect',lambda:bridge.job('connect')['ok']) is not True:
            raise RuntimeError('Cannot connect; stop rather than repeat failed operations')
        for u in ['fff1','fff3','fffa','fff2','fff4','fff5','fff6','fff7','fff8','fffb','ffe3','ffc1','ffd1','ffd2','ffd3']:
            b=check('read '+u,lambda u=u:bridge.read(u).hex())
            if b is not None:report['read'][u]=b
        check('device info',lambda:{k:v for k,v in bridge.job('info').items() if k in ('sn','version')})
        if a.controls:
            baseline={u:bytes.fromhex(v) for u,v in report['read'].items()}
            required=['fff1','fff3','fffa','fff2','fff4','fff5','fff8','fffb']
            if any(u not in baseline for u in required):raise RuntimeError('Baseline incomplete; no mutations performed')
            if int.from_bytes(baseline['fffb'],'big') or baseline['fff4'][0]:raise RuntimeError('Fan in Turbo/nature mode; preserve baseline and skip mutations')
            original_speed=baseline['fff3'][0];original_power=baseline['fff1'][0]
            try:
                for value in [min(100,max(1,original_speed+1)),21,original_speed or 10]:
                    check('speed '+str(value),lambda v=value: (bridge.job('speed',value=v),int.from_bytes(bridge.read('fff3'),'big'))[1])
                for value in [1,2,3,4,0]:check('light '+str(value),lambda v=value:(bridge.job('light',value=v),bridge.read('fffa')[0])[1])
                # Use the native fan timer, not a host-side approximation.
                check('timer 60 seconds',lambda:(bridge.write('fff2',(60).to_bytes(2,'big')),int.from_bytes(bridge.read('fff2'),'big'))[1])
                check('cancel timer',lambda:(bridge.write('fff2',b'\x00\x00'),bridge.read('fff2').hex())[1])
                check('sleep 60 seconds',lambda:(bridge.write('fff5',(60).to_bytes(2,'big')),int.from_bytes(bridge.read('fff5'),'big'))[1])
                check('Turbo duration 2 seconds',lambda:(bridge.write('fff8',b'\x00\x02'),int.from_bytes(bridge.read('fff8'),'big'))[1])
                check('Turbo on',lambda:bridge.write('fffc',b'\x01')['ok'])
                check('Turbo off',lambda:bridge.write('fffc',b'\x00')['ok'])
                check('nature on',lambda:(bridge.write('fff4',b'\x01'),bridge.read('fff4')[0])[1])
                check('nature off',lambda:(bridge.write('fff4',b'\x00'),bridge.read('fff4')[0])[1])
                for u,label in [('fff7','gear calibration unchanged'),('ffe3','128-point curve unchanged')]:
                    if u in baseline:check(label,lambda u=u:(bridge.write(u,baseline[u]),len(bridge.read(u)))[1])
            finally:
                check('restore Turbo off',lambda:bridge.write('fffc',b'\x00')['ok'])
                for u in ['fff4','fff8','fff5','fffa']:
                    check('restore '+u,lambda u=u:bridge.write(u,baseline[u])['ok'])
                # Restore the gear first, then its exact prior arbitrary speed.
                check('restore power',lambda:bridge.write('fff1',baseline['fff1'])['ok'])
                check('restore exact speed',lambda:bridge.write('fff3',baseline['fff3'])['ok'])
                timer=int.from_bytes(baseline['fff2'],'big')
                check('restore timer',lambda:bridge.write('fff2',timer.to_bytes(2,'big'))['ok'])
                check('final speed',lambda:bridge.read('fff3')[0])
                check('final light',lambda:bridge.read('fffa')[0])
                check('final power',lambda:bridge.read('fff1')[0])
    finally:
        bridge.port.close();out=Path(__file__).resolve().parents[1]/'dist'/'hardware-check.json';out.write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8');print('REPORT '+str(out),flush=True)
if __name__=='__main__':main()
