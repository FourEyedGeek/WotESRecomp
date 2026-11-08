"""Build a BPS extension for the verified Chili V2.0 cartridge. No ROM is packaged."""
from pathlib import Path
import struct,hashlib,zlib,json,argparse
ROOT=Path(__file__).resolve().parent
SOURCE_SHA1='2380b201b872f7bd2c66f2a4d82df86709332d55'
class Code:
 def __init__(self,base):self.base=base;self.data=bytearray();self.labels={};self.refs=[]
 @property
 def pc(self):return self.base+len(self.data)
 def emit(self,s):self.data.extend(bytes.fromhex(s));return self
 def word(self,n):self.data.extend(struct.pack('>H',n&65535));return self
 def long(self,n):self.data.extend(struct.pack('>I',n));return self
 def label(self,n):self.labels[n]=self.pc;return self
 def branch(self,opcode,label):self.word(opcode);self.refs.append((len(self.data),label));self.word(0);return self
 def jsr(self,a):return self.word(0x4eb9).long(a)
 def finish(self):
  for offset,label in self.refs:
   disp=self.labels[label]-(self.base+offset);assert -32768<=disp<=32767;struct.pack_into('>h',self.data,offset,disp)
  return bytes(self.data)
def small_icon(src,size):
 def pixel(x,y):
  b=src[((x//8)*2+y//8)*32+(y%8)*4+(x%8)//2];return (b>>(0 if x%2 else 4))&15
 out=bytearray(32)
 for y in range(size):
  for x in range(size):
   n=max(pixel(sx,sy) for sy in range(y*16//size,(y+1)*16//size) for sx in range(x*16//size,(x+1)*16//size));dx=x+(8-size)//2;dy=y+8-size;out[dy*4+dx//2]|=n<<(0 if dx%2 else 4)
 return bytes(out)
def build(source):
 assert len(source)==0x200000 and hashlib.sha1(source).hexdigest()==SOURCE_SHA1,'Requires the exact Chili V2.0 USA/Europe ROM; see README.'
 target=bytearray(source);changes=[]
 def write(a,b,why,old=None):
  if old is not None:assert source[a:a+len(old)]==old,(hex(a),'unexpected original instructions')
  if a>=0x110000:assert not any(source[a:a+len(b)]),'New helper area is occupied'
  target[a:a+len(b)]=b;changes.append({'address':f'{a:06X}','bytes':len(b),'purpose':why})
 def hook(a,dest,old):write(a,b'\x4e\xb9'+struct.pack('>I',dest),'Call cartridge helper',bytes.fromhex(old))
 # Fixed allocations keep source reconstruction reproducible.
 HAS=0x110000;FLAGS=0x110080;LABEL=0x110100;FLOOR=0x110180;LOAD=0x110200;SPRITE=0x110240
 TEXT=0x110280;ART=0x110300;UPLOADS=0x110400;DESCRIPTORS=0x110500
 c=Code(HAS).emit('2f02 2f09 43ea005c 7404')
 c.label('loop').emit('0c1100d6').branch(0x6600,'next').emit('4a290001').branch(0x6600,'found')
 c.label('next').emit('5489').branch(0x51ca,'loop').emit('7000').branch(0x6000,'done')
 c.label('found').emit('7001');c.label('done').emit('225f 241f 4e75');write(HAS,c.finish(),'Check all five active stench timers; preserve D2/A1')
 c=Code(FLAGS).emit('222a0068 2f00').jsr(HAS).emit('4a00').branch(0x6700,'done').emit('008140000000')
 c.label('done').emit('201f b29b 4e75');write(FLAGS,c.finish(),'Include stench presence in the display cache without modifying actor flags')
 c=Code(LABEL).emit('e54a 207c').long(0x64b6).emit('20702000 4a42').branch(0x6600,'done')
 c.emit('2f00 2f08').jsr(HAS).emit('4a00 205f').branch(0x6700,'restore').emit('41f9').long(TEXT)
 c.label('restore').emit('201f');c.label('done').emit('4e75');write(LABEL,c.finish(),'Show STENCH only when no higher-priority negative label is selected')
 hook(0x6344,FLAGS,'222a0068b29b');hook(0x648a,LABEL,'e54a207b2028');write(TEXT,b'STENCH\0','Six-character original-font condition label')
 write(0x100278,bytes.fromhex('488148c14e71'),'Sign-extend attack modifiers instead of zero-extending them',bytes.fromhex('0281000000ff'))
 icon=source[0xb86ee:0xb876e];assert len(icon)==128
 write(ART,icon+small_icon(icon,8)+small_icon(icon,6),'Inventory gauntlets artwork and two distance variants')
 c=Code(FLOOR).emit('0c2d0081df93').branch(0x6600,'original').emit('43f9').long(ART).emit('7800').branch(0x6000,'size')
 c.label('original').emit('eb4c');c.label('size').emit('3a3c0080 4e75');write(FLOOR,c.finish(),'Select gauntlets dungeon floor art using the actual item ID')
 hook(0x1fb60,FLOOR,'eb4c3a3c0080')
 # Extend the original sprite uploads; reserve the four unused tiles directly
 # after the original shared cube's four tiles. No other item is remapped.
 write(UPLOADS,source[0x1ea4c:0x1eaa4]+struct.pack('>IHH',ART,0x91e0,0x40),'Original eleven sprite uploads plus a separate four-tile gauntlet icon')
 c=Code(LOAD).emit('247c').long(UPLOADS).emit('740b 4e75');write(LOAD,c.finish(),'Load twelve sprite art records')
 hook(0x1ea2a,LOAD,'45fa0020740a')
 table=bytearray(source[0x1ebb6:0x1ec50]);assert len(table)==11*14
 gauntlet=bytearray(source[0x1ec42:0x1ec50]);gauntlet[10:12]=struct.pack('>H',0x48f);table+=gauntlet
 write(DESCRIPTORS,table,'Preserve original sprite descriptors; append a gauntlet-only descriptor')
 c=Code(SPRITE).emit('207c').long(DESCRIPTORS).emit('d0c1 4e75');write(SPRITE,c.finish(),'Resolve relocated sprite descriptor table')
 write(0x1eab0,b'\x4e\xb9'+struct.pack('>I',SPRITE)+b'\x4e\x71','Use extended sprite descriptor table',bytes.fromhex('41fa010441f01000'))
 write(0x1eac9+0x81,b'\x2c','Map only item129 to its own ground and pickup descriptor',b'\x28')
 # Sega checksum is the modulo-65536 sum of big-endian words after the header.
 checksum=sum(struct.unpack_from('>H',target,a)[0] for a in range(0x200,len(target),2))&65535
 write(0x18e,struct.pack('>H',checksum),'Update the Genesis header checksum')
 return bytes(target),{'source_sha1':SOURCE_SHA1,'source_crc32':f'{zlib.crc32(source):08x}','target_sha1':hashlib.sha1(target).hexdigest(),'target_crc32':f'{zlib.crc32(target):08x}','art_address':ART,'changes':changes}
def number(n):
 out=bytearray()
 while True:
  b=n&127;n>>=7
  if not n:out.append(b|128);return out
  out.append(b);n-=1
def bps(source,target):
 out=bytearray(b'BPS1')+number(len(source))+number(len(target))+number(0);i=0
 while i<len(target):
  start=i
  if source[i]==target[i]:
   while i<len(target) and source[i]==target[i]:i+=1
   out+=number((i-start-1)<<2)
  else:
   while i<len(target) and source[i]!=target[i]:i+=1
   out+=number(((i-start-1)<<2)|1)+target[start:i]
 out+=struct.pack('<II',zlib.crc32(source),zlib.crc32(target));out+=struct.pack('<I',zlib.crc32(out));return bytes(out)
def main():
 p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('--local-rom',type=Path,required=True);p.add_argument('--patch',type=Path,required=True);args=p.parse_args()
 assert args.local_rom.resolve()!=args.source.resolve(), "Choose a different output ROM filename"
 source=args.source.read_bytes();target,manifest=build(source);patch=bps(source,target)
 args.local_rom.parent.mkdir(exist_ok=True,parents=True);args.local_rom.write_bytes(target);args.patch.parent.mkdir(exist_ok=True,parents=True);args.patch.write_bytes(patch)
 (ROOT/'manifest.json').write_text(json.dumps(manifest,indent=2));print(json.dumps({k:v for k,v in manifest.items() if k!='changes'},indent=2));print('BPS bytes',len(patch))
if __name__=='__main__':main()
