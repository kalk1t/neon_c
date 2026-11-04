from pathlib import Path
import struct,json
p=Path(r'C:\Users\User\Projects\neon_c\win_app')
def glyph(b,cp):
 u16=lambda x:struct.unpack_from('>H',b,x)[0]
 u32=lambda x:struct.unpack_from('>I',b,x)[0]
 tables={b[12+i*16:16+i*16]:u32(20+i*16) for i in range(u16(4))};c=tables[b'cmap']
 for i in range(u16(c+2)):
  a=c+u32(c+8+i*8);fmt=u16(a)
  if fmt==12:
   for k in range(u32(a+12)):
    lo,hi,g=struct.unpack_from('>III',b,a+16+12*k)
    if lo<=cp<=hi:return g+cp-lo
  if fmt==4 and cp<=65535:
   n=u16(a+6)//2;ends=a+14;starts=ends+2*n+2;delta=starts+2*n;ranges=delta+2*n
   for k in range(n):
    lo,hi=u16(starts+2*k),u16(ends+2*k)
    if lo<=cp<=hi:
     d=u16(delta+2*k);r=u16(ranges+2*k)
     if not r:return (cp+d)&65535
     g=u16(ranges+2*k+r+2*(cp-lo));return (g+d)&65535 if g else 0
 return 0
print(json.dumps({f.name:{hex(c):glyph(f.read_bytes(),c) for c in [0xb7,0x2014,0x2248,0x265b]} for f in (p/'assets/ui-theme').glob('Orbitron-*.ttf')}))
