from pathlib import Path
import shutil,sys,struct,zlib,json,hashlib,argparse
from status_catalog032 import STATUSES
from community21 import Code,bps
parser=argparse.ArgumentParser(description='Build Complete Status v0.32 from a Community v2.2 ROM.')
parser.add_argument('source',type=Path)
parser.add_argument('output_dir',type=Path)
parser.add_argument('--previous',type=Path,help='Optional Status v0.31 ROM for the upgrade BPS.')
cli=parser.parse_args()
src=cli.output_dir;src.mkdir(parents=True,exist_ok=True)
base=cli.source.read_bytes()
assert zlib.crc32(base)==0x5c993724
target=bytearray(base)
LOOKUP=0x113000;TABLE=0x113100;TEXT=0x113200;SCREEN=0x113400
LIST=0x113600;DRAW=0x113A00;COORD=0x113D00;RANGE=0x113D40;APPEND=0x113DA0;COLLECT=0x113E00;CAT=0x114000;FSET=0x114800;IDMAP=0x114C00;STR=0x115000
parts=[]
def put(a,b):
 assert not any(base[a:a+len(b)]),hex(a)
 assert all(a+len(b)<=x or a>=x+n for x,n in parts),(hex(a),len(b),parts)
 target[a:a+len(b)]=b;parts.append((a,len(b)))
strings=bytearray()
def string(s):
 a=STR+len(strings);strings.extend(s.encode('ascii')+b'\0');return a
def positioned(lines,row=1):
 return lines[0]+''.join('\x03\x02'+chr(row+i)+s for i,s in enumerate(lines) if i and s)
records=[]
for title,mask,ids,description,kind in STATUSES:
 assert len(title)<=18,title
 assert all(len(s)<=26 for s in description.splitlines()),title
 name=None
 desc=string(positioned([title,'']+description.splitlines()));name=desc
 idptr=STR+len(strings);strings.extend(bytes(ids)+b'\xff')
 records.append(struct.pack('>IIIIHH',mask&0xffffffff,idptr,name,desc,kind,0))
fallback=CAT+len(records)*20
records.append(struct.pack('>IIIIHH',0,0,string('UNKNOWN'),string(positioned(['UNKNOWN EFFECT','','UNRECOGNISED EFFECT ID.','NO ASSUMED STAT MODIFIER.'])),0,0))
empty=string(positioned(['NO ACTIVE EFFECTS','','NO RECOGNISED CONDITIONS','OR TIMED EFFECTS FOUND.']))
heading=string(positioned(['STATUS              TIME'],9))
put(CAT,b''.join(records));put(STR,bytes(strings))
# Maps cover every original record, including the later Chili opcodes. The
# covered mask avoids showing both a generic flag and its named gear source.
audit=[]
lengths=base[0x13b54:0x13b8c]
for i in range(236):
 program=base[0x3e58c+i*16+9:0x3e58c+(i+1)*16];ops=[];j=0
 while j<len(program) and program[j]:
  op=program[j]&127;j+=1
  if op>=0x37:break # Non-dispatched padding/sentinel, e.g. healing potion FF.
  n=lengths[op];args=list(program[j:j+n]);assert len(args)==n,(i,op)
  ops.append({'op':op,'args':args});j+=n
 audit.append({'id':i,'decoded':ops})
flagsets=[0]*256
for item in audit:
 for op in item['decoded']:
  if op['op']==4:flagsets[item['id']]|=1<<op['args'][0]
  if op['op']==0x28:flagsets[item['id']]|=0x3600
  if op['op'] in [0x27,0x2f]:flagsets[item['id']]|=1<<19
  if op['op']==0x29:flagsets[item['id']]|=1<<5
  if op['op']==0x35:flagsets[item['id']]|=1<<26
put(FSET,b''.join(struct.pack('>I',x) for x in flagsets))
idmap=bytearray(512)
for name,mask,ids,desc,kind in STATUSES:
 for i in ids:idmap[kind*256+i]=1
put(IDMAP,bytes(idmap))
put(LOOKUP,Code(LOOKUP).emit('5340 e540 207c').long(TABLE).emit('20700000 4e75').finish())
target[0xa410:0xa418]=bytes.fromhex('4eb9')+struct.pack('>I',LOOKUP)+bytes.fromhex('4e71')
put(TABLE,b''.join(struct.pack('>I',a) for a in [0xa4c0,0xa4e2,0xa4fe,0xa534,SCREEN,0xa49c]))
target[0xa3dc:0xa3de]=bytes.fromhex('7406');target[0xa3e6:0xa3e8]=bytes.fromhex('7407')
target[0xa468:0xa46c]=struct.pack('>I',TEXT);target[0xa488:0xa48a]=bytes.fromhex('0516')
put(TEXT,b'OPTIONS\x03\x05\x04RETURN TO CAMP\x03\x05\x07DROP AN ITEM\x03\x05\x0ACHANGE ROSTER\x03\x05\x0DMUSIC IS \x03\x05\x10BAR GRAPHS ARE\x03\x05\x13STATUS EFFECTS\0')

# One screen handler; no retained copies of previous preview handlers.
# Stack-local workspace: selection/count/previous keys, actor, matched slots,
# 64 entries of {metadata pointer, timer, original effect ID}, and a text buffer.
c=Code(SCREEN).emit('48e7 fffe 4fef fc00 284f 426c0000 426c0004').jsr(0xb3aa).jsr(LIST).jsr(DRAW)
c.emit('41f9').long(0x4da14).jsr(0xb3a4)
c.label('loop').jsr(COORD).jsr(0x6308).jsr(0x614c).jsr(0xa8b6)
c.emit('322c0004 39400004 b141 c240 0800 0007').branch(0x6600,'exit')
c.emit('0801 0005').branch(0x6700,'up')
c.emit('5238 8f46 0c38 0005 8f46').branch(0x6600,'party').emit('11fc 0001 8f46')
c.label('party').emit('426c0000').jsr(LIST).branch(0x6000,'redraw')
c.label('up').emit('4a6c0002').branch(0x6700,'loop').emit('0801 0000').branch(0x6700,'down')
c.emit('536c0000').branch(0x6a00,'redraw').emit('396c00020000 536c0000').branch(0x6000,'redraw')
c.label('down').emit('0801 0001').branch(0x6700,'loop').emit('526c0000 302c0000 b06c0002').branch(0x6d00,'redraw').emit('426c0000')
c.label('redraw').jsr(DRAW).branch(0x6000,'loop')
c.label('exit').jsr(0xa8b6).emit('0800 0007').branch(0x6600,'exit')
c.jsr(0xb3aa).jsr(0xa45c).emit('41f9').long(0x4da14).jsr(0xb3a4).emit('4fef0400 4cdf7fff 4e75')
put(SCREEN,c.finish())

# Collect only records affecting this companion, then resolve display metadata.
c=Code(LIST).emit('48e7fffe').jsr(COLLECT).emit('266c0008 43ec0020 7c00 3e3c').word(len(STATUSES)-1).emit('45f9').long(CAT)
c.label('record').emit('7aff 2012 0c80ffffffff').branch(0x6600,'flags')
c.emit('4a2b0027').branch(0x6600,'sources').emit('7afe').branch(0x6000,'sources')
c.label('flags').emit('0c8000000200').branch(0x6700,'vision').emit('0c8000000400').branch(0x6600,'testflags')
c.label('vision').emit('222c000c 028100003000').branch(0x6600,'sources')
c.label('testflags').emit('c0ac000c 222c0010 4681 c081').branch(0x6700,'sources').emit('7afe')
c.label('sources').emit('342c0006 5342').branch(0x6b00,'finishrecord').emit('41ec0280')
c.label('source').emit('7600 16280001 b66a0010').branch(0x6600,'nextsource').emit('1610 2f08 206a0004')
c.label('alias').emit('1018 0c0000ff').branch(0x6700,'nomatch').emit('b003').branch(0x6600,'alias')
c.emit('205f 30280002 b045').branch(0x6e00,'timer').emit('4a45').branch(0x6a00,'nextsource')
c.label('timer').emit('3a00').branch(0x6000,'nextsource')
c.label('nomatch').emit('205f')
c.label('nextsource').emit('5888').branch(0x51ca,'source')
c.label('finishrecord').emit('0c45ffff').branch(0x6700,'nextrecord').emit('0c460040').branch(0x6c00,'done')
c.emit('22ca 32c5 32fc0000 5246')
c.label('nextrecord').emit('d4fc0014').branch(0x51cf,'record')
c.emit('342c0006 5342').branch(0x6b00,'done').emit('41ec0280')
c.label('unknown').emit('7000 1010 7200 12280001 c2fc0100 d041 45f9').long(IDMAP).emit('4a320000').branch(0x6600,'nextunknown')
c.emit('0c460040').branch(0x6c00,'done').emit('22fc').long(fallback).emit('32e80002 7000 1010 32c0 5246')
c.label('nextunknown').emit('5888').branch(0x51ca,'unknown')
c.label('done').emit('39460002 4cdf7fff 4e75');put(LIST,c.finish())

c=Code(DRAW).emit('48e7fffe 7001').jsr(0x877c).jsr(0xaa0e)
for a,row in [(heading,9)]:c.emit('41f9').long(a).emit('203c').long(0x02000000|(row<<16)).jsr(0xa67a)
c.emit('4a6c0002').branch(0x6600,'content').emit('41f9').long(empty).emit('203c02010000').jsr(0xa67a).branch(0x6000,'done')
c.label('content').emit('7000 302c0000 e748 47ec0020 d6c0 2053 2068000c 203c02010000').jsr(0xa67a)
c.emit('7000 302c0000 80fc0006 02800000ffff c0fc0006 3800 263c020a0000 7e05')
c.label('row').emit('b86c0002').branch(0x6c00,'done')
c.emit('7000 3004 e748 47ec0020 d6c0 2453 41ec0240 720b')
# Clear 24 characters and terminate the line.
c.label('spaces').emit('30fc2020').branch(0x51c9,'spaces').emit('4210 41ec0241 226a0008')
c.label('name').emit('1019').branch(0x6700,'timer').emit('0c000003').branch(0x6700,'timer').emit('10c0').branch(0x6000,'name')
c.label('timer').emit('302b0004').branch(0x6a00,'digits').emit('397c2d2d0254').branch(0x6000,'marker')
c.label('digits').emit('80fc0064 3200 06410030 19410254 4840 02800000ffff 80fc000a 3200 06410030 19410255 4840 06400030 19400256')
c.emit('0c2c00300254').branch(0x6600,'marker').emit('197c00200254 0c2c00300255').branch(0x6600,'marker').emit('197c00200255')
c.label('marker').emit('b86c0000').branch(0x6600,'print').emit('197c003e0240')
c.label('print').emit('41ec0240 2003').jsr(0xa67a).emit('5244 068300020000').branch(0x51cf,'row')
c.label('done').emit('4cdf7fff 4e75');put(DRAW,c.finish())
# Reuse the original portrait-coordinate table without its selection sound.
put(COORD,base[0x83c:0x850]+bytes.fromhex('11fc0001a3b8 4e75'))
# Pure range predicate matching the original aura helpers. D4 carries the
# original cave alignment mask (party sources use FFF8, ground auras FFFC).
c=Code(RANGE).emit('4a38a93b').branch(0x6700,'outdoors')
c.emit('c084 c284 b280').branch(0x6600,'outside').branch(0x6000,'inside')
c.label('outdoors').jsr(0x28da).emit('7200 12280006 e349 b041').branch(0x6e00,'outside')
c.label('inside').emit('7001 4e75');c.label('outside').emit('7000 4e75');put(RANGE,c.finish())

# Append a normalized {zero-based ID, source kind, remaining timer} record.
c=Code(APPEND).emit('48e7f8a0 0c460050').branch(0x6c00,'done')
c.emit('12c0 12c1 32c5 5246 e548 41f9').long(FSET).emit('24300000 85ac0010')
c.label('done').emit('4cdf051f 4e75');put(APPEND,c.finish())

c=Code(COLLECT).emit('48e7fffe 7000 10388f46 5340 c0fc0096 47f8912c d7c0 294b0008 202b0068 2940000c 42ac0010 43ec0280 7c00')
# Silence gates party spell selection through the aggregate party flags.
c.emit('45f8912c 7e03')
c.label('partyflags').emit('4a2a0027').branch(0x6700,'nextflags')
c.emit('202a0068 028000000800 81ac000c')
c.label('nextflags').emit('45ea0096').branch(0x51cf,'partyflags')
# Personal timed effects, including the newest potion/monster records.
c.emit('41eb005c 7e04')
c.label('personal').emit('7a00 1a280001').branch(0x6700,'nextpersonal')
c.emit('7000 1010 7200').jsr(APPEND)
c.label('nextpersonal').emit('5488').branch(0x51cf,'personal')
# Party-shared timed effects. Preserve the actual cartridge's unscaled
# record-category lookup at $135FE rather than silently fixing its rules.
c.emit('45f8912c 7603')
c.label('member').emit('b5cb').branch(0x6700,'nextmember').emit('41ea005c 7404')
c.label('partyslot').emit('7a00 1a280001').branch(0x6700,'nextparty')
c.emit('7000 1010 2f08 41f9').long(0x3e58c).emit('0c3000020000').branch(0x6600,'partyoutside')
c.emit('e948 d0c0 202b0020 222a0020 283cfff8fff8').jsr(RANGE).emit('4a00').branch(0x6700,'partyoutside')
c.emit('205f 7000 1010 7200').jsr(APPEND).branch(0x6000,'nextparty')
c.label('partyoutside').emit('205f')
c.label('nextparty').emit('5488').branch(0x51ca,'partyslot')
c.label('nextmember').emit('45ea0096').branch(0x51cb,'member')
# Twelve world/ground aura records, with exactly the original range checks.
c.emit('41f88ce4 45f88cfc 7e0b')
c.label('aura').emit('7a00 1a280001').branch(0x6700,'nextaura')
c.emit('7000 1010 e948 2f08 41f9').long(0x3e58c).emit('d0c0 202b0020 2212 283cfffcfffc').jsr(RANGE).emit('4a00').branch(0x6700,'auraoutside')
c.emit('205f 7000 1010 7200').jsr(APPEND).branch(0x6000,'nextaura')
c.label('auraoutside').emit('205f')
c.label('nextaura').emit('5488 588a').branch(0x51cf,'aura')
# The same inventory/ability scan used by $136CA for magic item types 6/7.
c.emit('41eb002c 7e25')
c.label('gear').emit('7000 1010').branch(0x6700,'nextgear').emit('5340 0c4000ec').branch(0x6400,'nextgear')
c.emit('2f08 3400 e94a 41f9').long(0x3e58c).emit('d0c2 0c100006').branch(0x6700,'gearrecord')
c.emit('0c100007').branch(0x6600,'gearskip')
c.label('gearrecord').emit('4a280009').branch(0x6700,'gearskip')
c.emit('205f 7201 7afe').jsr(APPEND).branch(0x6000,'nextgear')
c.label('gearskip').emit('205f')
c.label('nextgear').emit('5288').branch(0x51cf,'gear')
c.emit('39460006 4cdf7fff 4e75');put(COLLECT,c.finish())
struct.pack_into('>H',target,0x18e,sum(struct.unpack_from('>H',target,a)[0] for a in range(0x200,len(target),2))&65535)
target=bytes(target);crc=zlib.crc32(target)
(src/'local').mkdir(exist_ok=True);(src/'local/wotes.md').write_bytes(target)
out=src
patch=bps(base,target);(out/'WotES Complete Status v0.32 - from Community v2.2.bps').write_bytes(patch)
manifest={'target_crc32':f'{crc:08X}','target_sha1':hashlib.sha1(target).hexdigest(),'patch_bytes':len(patch),'known_status_entries':len(STATUSES),'parts':[(hex(a),n) for a,n in parts]}
(src/'local/status-menu-manifest.json').write_text(json.dumps(manifest,indent=2));print(manifest)




assert b'UP/DOWN: INFO' not in target and b'TIME: TICKS / -- NO TIMER' not in target
if cli.previous:
 previous=cli.previous.read_bytes()
 assert zlib.crc32(previous)==0x9df03069
 (out/'WotES Complete Status v0.32 - from Status v0.31.bps').write_bytes(bps(previous,target))
 assert all(a==b or (0x18e<=i<0x190) or (0x113000<=i<0x118000) for i,(a,b) in enumerate(zip(previous,target)))
print('Built display-only revision. Keep local/wotes.md private; distribute BPS files only.')
