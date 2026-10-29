"""Listen only: record speed commands, never read secrets or send control."""
import json,time
from pathlib import Path
import serial
root=Path(__file__).resolve().parents[1]
port=serial.Serial('COM12',115200,timeout=.25)
try:
    end=time.monotonic()+55
    with (root/'dist/speed-commands.jsonl').open('a',encoding='utf-8') as out:
        print('LISTENING FOR 55 SECONDS',flush=True)
        while time.monotonic()<end:
            line=port.readline().decode(errors='replace').strip()
            if line.startswith(('CLOUD_SPEED_RX','SPEED_WRITE')):
                record={'time':time.strftime('%Y-%m-%d %H:%M:%S'),'line':line}
                out.write(json.dumps(record,ensure_ascii=False)+'\n');out.flush();print(line,flush=True)
finally:port.close()
