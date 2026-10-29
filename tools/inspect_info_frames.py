"""Inspect actual captured W96P information frames without operating hardware."""
import re
from pathlib import Path
text=(Path(__file__).resolve().parents[1]/'include/vendor_crc.h').read_text()
table=list(map(int,re.findall(r'\d+',text.split('={',1)[1])))
for h in ['55b7d6d4d0c6a8','55069f9a9022129b9a21']:
    raw=bytes.fromhex(h);mask=table[raw[1]]
    decoded=raw[:2]+bytes(v^mask for v in raw[2:])
    print('raw',raw.hex(),'decoded',decoded.hex(),'mask',hex(mask))
    for mode,data in [('encoded',raw[:-1]),('decoded',decoded[:-1])]:
        crc=137
        for v in data:crc=table[crc^v]
        print(mode,hex(crc),'rawcrc',hex(raw[-1]),'decodedcrc',hex(decoded[-1]))
