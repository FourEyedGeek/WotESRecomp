"""Rebuild Community v2.3 from an exact Chili V2.0 ROM; no ROM is distributed."""
from pathlib import Path
import argparse,hashlib,json,subprocess,sys,tempfile,zlib
from build_patch22 import build as build22
from community21 import bps
from apply_bps import apply
HERE=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('--output-rom',type=Path,required=True);p.add_argument('--patch-dir',type=Path,required=True);a=p.parse_args()
assert not a.output_rom.exists(),'Choose a new output ROM filename'
assert a.source.resolve()!=a.output_rom.resolve(),'Never overwrite the input ROM'
source=a.source.read_bytes();_,base,earlier=build22(source)
assert zlib.crc32(base)==0x5c993724
a.output_rom.parent.mkdir(parents=True,exist_ok=True)
with tempfile.TemporaryDirectory(prefix='wotes-v23-',dir=a.output_rom.parent.resolve()) as td:
 tmp=Path(td);assert tmp.resolve().is_relative_to(a.output_rom.parent.resolve())
 (tmp/'v22.md').write_bytes(base)
 subprocess.run([sys.executable,str(HERE/'build_status_audit032.py'),str(tmp/'v22.md'),str(tmp/'status')],check=True)
 target=(tmp/'status/local/wotes.md').read_bytes()
assert zlib.crc32(target)==0x91efc7f8 and hashlib.sha1(target).hexdigest()=='8757cf0e935656d98e1a29677e3e15ba069aa0e5'
patch=bps(source,target);assert apply(source,patch)==target
name='WotES Community v2.3 - APPLY TO Chili V2.0.bps'
a.output_rom.parent.mkdir(parents=True,exist_ok=True);a.output_rom.write_bytes(target)
a.patch_dir.mkdir(parents=True,exist_ok=True);(a.patch_dir/name).write_bytes(patch)
manifest={'version':'v2.3','status_screen_revision':'v0.32','source_crc32':f'{zlib.crc32(source):08X}','target_crc32':f'{zlib.crc32(target):08X}','source_sha1':hashlib.sha1(source).hexdigest(),'target_sha1':hashlib.sha1(target).hexdigest(),'bytes':len(target),'patch_bytes':len(patch),'patch_sha256':hashlib.sha256(patch).hexdigest(),'cumulative':True,'earlier_fix_manifest':earlier}
(a.patch_dir/'manifest.json').write_text(json.dumps(manifest,indent=2))
print(json.dumps({k:v for k,v in manifest.items() if k!='earlier_fix_manifest'},indent=2))
