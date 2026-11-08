"""Independent BPS application and checksum validation. Never modifies its input."""
from pathlib import Path
import struct,zlib,argparse

def apply(source,patch):
 if patch[:4]!=b'BPS1' or len(patch)<19:raise ValueError('Not a BPS patch')
 sc,tc,pc=struct.unpack_from('<III',patch,len(patch)-12)
 if zlib.crc32(patch[:-4])!=pc:raise ValueError('Patch checksum mismatch')
 if zlib.crc32(source)!=sc:raise ValueError('Wrong input ROM: this patch requires Chili V2.0')
 pos=4
 def read_number():
  nonlocal pos
  value=0;multiplier=1
  while True:
   if pos>=len(patch)-12:raise ValueError('Truncated patch')
   byte=patch[pos];pos+=1;value+=(byte&127)*multiplier
   if byte&128:return value
   multiplier*=128;value+=multiplier
 source_size=read_number();target_size=read_number();meta_size=read_number();pos+=meta_size
 if source_size!=len(source):raise ValueError('Wrong source size')
 output=bytearray();source_cursor=target_cursor=0
 while pos<len(patch)-12:
  instruction=read_number();count=(instruction>>2)+1;command=instruction&3
  if command==0:
   start=len(output)
   if start+count>len(source):raise ValueError('Source read outside ROM')
   output.extend(source[start:start+count])
  elif command==1:
   if pos+count>len(patch)-12:raise ValueError('Truncated literal')
   output.extend(patch[pos:pos+count]);pos+=count
  else:
   delta=read_number();delta=-(delta>>1) if delta&1 else delta>>1
   if command==2:
    source_cursor+=delta
    if source_cursor<0 or source_cursor+count>len(source):raise ValueError('Invalid source copy')
    output.extend(source[source_cursor:source_cursor+count]);source_cursor+=count
   else:
    target_cursor+=delta
    for _ in range(count):
     if not 0<=target_cursor<len(output):raise ValueError('Invalid target copy')
     output.append(output[target_cursor]);target_cursor+=1
  if len(output)>target_size:raise ValueError('Patch exceeds target size')
 if len(output)!=target_size or zlib.crc32(output)!=tc:raise ValueError('Patched ROM checksum mismatch')
 return bytes(output)
if __name__=='__main__':
 parser=argparse.ArgumentParser();parser.add_argument('source',type=Path);parser.add_argument('patch',type=Path);parser.add_argument('output',type=Path);a=parser.parse_args()
 if a.output.exists():raise SystemExit('Output already exists; choose a new filename.')
 result=apply(a.source.read_bytes(),a.patch.read_bytes());a.output.write_bytes(result);print('Created',a.output)
