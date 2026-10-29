"""Recorded hardware frames pin CRC decoding independently of live hardware."""
import re,unittest
from pathlib import Path
text=(Path(__file__).resolve().parents[1]/'include/vendor_crc.h').read_text()
table=list(map(int,re.findall(r'\d+',text.split('={',1)[1])))
def decode(h):
    raw=bytes.fromhex(h);mask=table[raw[1]];d=raw[:2]+bytes(v^mask for v in raw[2:])
    length=int.from_bytes(d[2:4],'little')
    if len(d)!=length+5:raise ValueError('length')
    crc=137
    for v in d[:-1]:crc=table[crc^v]
    if crc!=d[-1]:raise ValueError('CRC')
    return d[4:-1]
class Frames(unittest.TestCase):
    def test_entire_polynomial_table(self):
        for i in range(256):
            c=i
            for _ in range(8):c=((c<<1)^0x3b)&255 if c&128 else (c<<1)&255
            self.assertEqual(table[i],c,f'entry {i}')
    def test_recorded_version(self):
        self.assertEqual(decode('55b7d6d4d0c6a8'),bytes.fromhex('0412'))
    def test_recorded_serial(self):
        p=decode('55069f9a9022129b9a21')
        self.assertEqual(p[0],10);self.assertEqual(int.from_bytes(p[1:],'little'),100536)
if __name__=='__main__':unittest.main()
