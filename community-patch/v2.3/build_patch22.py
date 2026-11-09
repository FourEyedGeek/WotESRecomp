"""Reproduce Community v2.2 from an exact PlanetChili V2.0 ROM."""
from pathlib import Path
import argparse,struct,hashlib,zlib,json
from community21 import build as build21,bps

def build(source):
    previous,manifest21=build21(source)
    target=bytearray(previous);changes=[]
    def write(address,data,reason,expected=None):
        if expected is not None:
            assert target[address:address+len(expected)]==expected,hex(address)
        if address>=0x110600:
            assert not any(target[address:address+len(data)]),'Helper area occupied'
        target[address:address+len(data)]=data
        changes.append({'address':f'{address:06X}','bytes':len(data),'purpose':reason})
    def jump(address,destination,original):
        write(address,bytes.fromhex('4ef9')+struct.pack('>I',destination),'Jump to cartridge action-preservation helper',bytes.fromhex(original))
    # A word-sized stack backup preserves the action byte without changing
    # registers or allocating global RAM. JMP entry avoids an extra JSR frame.
    # Original MOVE.B #$FF,$A8F7 follows, with its original flags and fallthrough.
    write(0x110600,bytes.fromhex('3f38 a8f8 11fc 00ff a8f7 4ef9 0000 3956'),
          'Save action scratch word on stack; reproduce targeting-entry instruction')
    # Byte postincrement on A7 advances TWO bytes on a plain MC68000.
    # Restore only A8F8: the adjacent A8F9 action/result byte is NOT restored.
    # Both original exits have identical restore-call/MOVEM/RTS tails.
    write(0x110640,bytes.fromhex('11df a8f8 11fc 0000 a8f7 4ef9 0000 3b68'),
          'Restore saved action byte, balance stack, reproduce original exit flag and tail')
    jump(0x3950,0x110600,'11fc00ffa8f7')
    jump(0x3b62,0x110640,'11fc0000a8f7')
    jump(0x3d7e,0x110640,'11fc0000a8f7')
    checksum=sum(struct.unpack_from('>H',target,a)[0] for a in range(0x200,len(target),2))&65535
    target[0x18e:0x190]=struct.pack('>H',checksum)
    result=bytes(target)
    def fingerprint(b):return {'bytes':len(b),'crc32':f'{zlib.crc32(b):08X}','sha1':hashlib.sha1(b).hexdigest(),'sha256':hashlib.sha256(b).hexdigest()}
    return previous,result,{'base_chili_v2':fingerprint(source),'community_v21':fingerprint(previous),'community_v22':fingerprint(result),'changes_from_v21':changes,'genesis_checksum':f'{checksum:04X}'}

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('--output-rom',type=Path,required=True);p.add_argument('--patch-dir',type=Path,required=True);a=p.parse_args()
    assert a.source.resolve()!=a.output_rom.resolve(),'Choose a new ROM filename'
    assert not a.output_rom.exists(),'Output ROM already exists'
    source=a.source.read_bytes();previous,target,manifest=build(source)
    a.output_rom.parent.mkdir(exist_ok=True,parents=True);a.output_rom.write_bytes(target)
    a.patch_dir.mkdir(exist_ok=True,parents=True)
    (a.patch_dir/'WotES Community v2.2 - APPLY TO Chili V2.0.bps').write_bytes(bps(source,target))
    (a.patch_dir/'WotES Community v2.2 - UPGRADE FROM Community v2.1.bps').write_bytes(bps(previous,target))
    (a.patch_dir/'manifest.json').write_text(json.dumps(manifest,indent=2))
    print(json.dumps(manifest,indent=2))
