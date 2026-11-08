/* Run cartridge stat helpers on a private CPU/RAM copy. No live bus,
 * game hooks, interrupts, audio, time or random generator are advanced. */
#include "m68k_interp.h"
#include "wotes_shop.h"
#include "wotes_rules.h"
#include <string.h>
static M68KState eval_cpu;
static uint8_t eval_ram[65536], eval_sram[16];
static int eval_bad, eval_roll=1, eval_high;
static int eval_preparing, eval_prepare_rolls, eval_prepare_unsupported;
static uint8_t eval_read8(uint32_t a) {
 a&=0xffffff;
 if(a>=0xff0000)return eval_ram[a&65535];
 if(a<0x200000)return g_rom[a];
 if(a>=0x204000 && a<0x204010)return eval_sram[a-0x204000];
 eval_bad=1;return 0;
}
static uint16_t eval_read16(uint32_t a){return (eval_read8(a)<<8)|eval_read8(a+1);}
static uint32_t eval_read32(uint32_t a){return ((uint32_t)eval_read16(a)<<16)|eval_read16(a+2);}
static void eval_write8(uint32_t a,uint8_t v){
 a&=0xffffff;
 if(a>=0xff0000)eval_ram[a&65535]=v;
 else if(a>=0x204000 && a<0x204010)eval_sram[a-0x204000]=v;
 else eval_bad=1;
}
static void eval_write16(uint32_t a,uint16_t v){eval_write8(a,(uint8_t)(v>>8));eval_write8(a+1,(uint8_t)v);}
static void eval_write32(uint32_t a,uint32_t v){eval_write16(a,v>>16);eval_write16(a+2,v);}
#define g_cpu eval_cpu
#define g_ram eval_ram
#define m68k_read8 eval_read8
#define m68k_read16 eval_read16
#define m68k_read32 eval_read32
#define m68k_write8 eval_write8
#define m68k_write16 eval_write16
#define m68k_write32 eval_write32
#define g_m68ki_bad_pc shop_private_g_m68ki_bad_pc
#define g_m68ki_bad_op shop_private_g_m68ki_bad_op
#define g_m68ki_insn_count shop_private_g_m68ki_insn_count
#define g_m68ki_discover shop_private_g_m68ki_discover
#define g_m68ki_discover_count shop_private_g_m68ki_discover_count
#define m68k_interp_cov_active shop_private_m68k_interp_cov_active
#define m68k_interp_cov_dump shop_private_m68k_interp_cov_dump
#define m68k_interp_step shop_private_m68k_interp_step
#define m68k_interp_run shop_private_m68k_interp_run
#define m68k_interp_run_framed shop_private_m68k_interp_run_framed
#define m68k_interp_run_handler shop_private_m68k_interp_run_handler
#define m68k_interp_run_ram_handler shop_private_m68k_interp_run_ram_handler
#include "m68k_interp.c"
static int stored_effect_check(void){
 /* The effect interpreter keeps the current record's end in A5.
  * Only bypass a resistance check for an effect already stored on its
  * target; equipment procs and unknown random preparation stay unsupported. */
 unsigned end=eval_cpu.A[5]&0xffffff,target=eval_cpu.A[4]&0xffffff;
 if(end<0x3e59c || end>0x3e58c+236*16 || ((end-0x3e58c)&15) || target<0xff0000)return 0;
 unsigned id=(end-0x3e58c)/16-1,a=target&65535,operand=eval_cpu.A[2]&0xffffff;
 if(a>65536-150 || operand<end-7 || operand>=end)return 0;
 for(unsigned k=92;k<102;k+=2)if(eval_ram[a+k]==id && eval_ram[a+k+1])return 1;
 return 0;
}
/* Use the shared instruction semantics directly, omitting scheduler/hook tails.
 * Only the audited helper ranges below may execute; any surprise fails closed. */
static int calculate(uint32_t pc,uint32_t stop) {
 eval_cpu.PC=pc;eval_cpu.A[7]=0xfff000;
 eval_write32(0xfff000,0x1ffffe);
 for(unsigned guard=0;guard<30000 && !eval_bad;++guard) {
  pc=eval_cpu.PC&0xffffff;
  if(pc==0x1ffffe || (stop && pc==stop))return 1;
  /* Apply the combined currently stored effects without rerolling their
   * acquisition/resistance checks. This is only a private display model. */
  if(eval_preparing && (pc==0x13f96 || pc==0x13f90) && stored_effect_check()){
   if(pc==0x13f96)++eval_cpu.A[2];
   eval_cpu.D[0]=0xffff;
   eval_cpu.SR=(eval_cpu.SR&~15u)|8u;
   eval_cpu.PC=eval_read32(eval_cpu.A[7]);eval_cpu.A[7]+=4;continue;
  }
  if(pc==0xa646) {
   unsigned n=eval_cpu.D[0]&255,sides=eval_cpu.D[1]&255;
   int bonus=eval_cpu.A[0]&255;
   if(eval_preparing) {
    ++eval_prepare_rolls;
    if(n!=1 || sides!=20)eval_prepare_unsupported=1;
   }
   eval_cpu.D[0]=n*(sides==20?eval_roll:eval_high?sides:1)+bonus;
   eval_cpu.PC=eval_read32(eval_cpu.A[7]);eval_cpu.A[7]+=4;continue;
  }
  if(!((pc>=0x13000 && pc<0x15500)||(pc>=0x100000 && pc<0x101800)))return 0;
  M68KInstr ins;uint32_t next;
  if(!m68k_decode(busview(),pc,&ins))return 0;
  s_illegal_ea=0;
  if(exec_one(&ins,&next)!=M68KI_OK || s_illegal_ea)return 0;
  eval_cpu.PC=next;
 }
 return 0;
}
static unsigned record(unsigned id){return 0x3e58c+(id-1)*16;}
static void prepare(const uint8_t *ram,unsigned actor) {
 memcpy(eval_ram,ram,65536);memset(&eval_cpu,0,sizeof eval_cpu);
 memset(eval_sram,0,sizeof eval_sram);eval_bad=0;eval_cpu.SR=0x2700;
 eval_roll=1;eval_high=0;eval_preparing=1;
 eval_prepare_rolls=eval_prepare_unsupported=0;
 eval_cpu.A[0]=0xff0000|actor;
}
int wotes_shop_ac(const uint8_t *ram,unsigned actor,unsigned item,int *out) {
 prepare(ram,actor);
 if(item) {
  unsigned kind=g_rom[record(item)],slot=44;
  for(unsigned k=44;k<52;++k) {
   unsigned old=eval_ram[actor+k];
   if(old && g_rom[record(old)]==kind){eval_ram[actor+k]=0;slot=k;}
  }
  eval_ram[actor+slot]=(uint8_t)item;
 }
 if(!calculate(0x13810,0))return 0;
 eval_cpu.A[0]=0xff0000|actor;
 if(!calculate(0x13510,0))return 0;
 if(eval_prepare_rolls)return 0;
 *out=(int8_t)eval_ram[actor+103];return !eval_bad;
}
int wotes_shop_weapon(const uint8_t *ram,unsigned actor,unsigned item,int *low,int *high,int *hit,int *status) {
 if(!item || g_rom[record(item)]!=2)return 0;
 /* Conditional enchantments need a particular enemy. Don't substitute an
  * ordinary weapon's hit formula for a different cartridge effect program. */
 if(g_rom[record(item)+9]!=0x1e || g_rom[record(item)+11]!=0x10 ||
    g_rom[record(item)+12]!=2)return 0;
 *low=255;*high=0;*hit=0;*status=0;
 for(unsigned n=93;n<102;n+=2)if(ram[actor+n])*status=1;
 prepare(ram,actor);
 memset(eval_ram+actor+108,0,3);
 if(!calculate(0x13810,0))return 0;
 eval_cpu.A[0]=0xff0000|actor;
 if(!calculate(0x13510,0))return 0;
 /* Unknown random-dependent preparation cannot be presented as a current
  * fixed rating. Checks for already stored conditions are handled above. */
 if(eval_prepare_unsupported || eval_prepare_rolls)return 0;
 eval_preparing=0;
 /* Ordinary unenchanted opponent, AC 0; all other enemy effects are absent. */
 unsigned target=0xe800;memset(eval_ram+target,0,150);
 eval_ram[target]=0x80;eval_ram[target+39]=eval_ram[target+40]=200;
 /* $406A/$40A0 store the zero-based cartridge record index here.
  * Passing the inventory ID misclassifies the last bow/sling variant. */
 eval_ram[0xa4cf]=(uint8_t)(item-1);eval_ram[0xa4ce]=2;
 eval_ram[0xa6d0]=0;
 static uint8_t prepared[65536];memcpy(prepared,eval_ram,sizeof prepared);
 M68KState prepared_cpu=eval_cpu;
 int chance=0;
 for(int roll=1;roll<=20;++roll) {
  memcpy(eval_ram,prepared,sizeof prepared);eval_cpu=prepared_cpu;
  eval_roll=roll;eval_cpu.A[3]=0xff0000|actor;eval_cpu.A[4]=0xff0000|target;
  eval_cpu.A[2]=record(item)+10;eval_cpu.D[0]=0;eval_cpu.SR=0x2700;
  if(!calculate(0x1427e,0))return 0;
  if((int16_t)eval_cpu.D[0]!=0)chance+=5;
 }
 for(int h=0;h<2;++h) {
  memcpy(eval_ram,prepared,sizeof prepared);eval_cpu=prepared_cpu;
  eval_high=h;eval_cpu.A[3]=0xff0000|actor;eval_cpu.A[4]=0xff0000|target;
  eval_cpu.A[2]=record(item)+13;eval_ram[0xa6d1]=0;
  if(!calculate(0x13cec,0x13d40))return 0;
  int damage=eval_cpu.D[0]&255;if(damage<1)damage=1;
  if(damage<*low)*low=damage;
  if(damage>*high)*high=damage;
 }
 /* Nominal d20 rating with current modifiers: every face counts as 5%.
  * This is not a prediction of the cartridge PRNG's uneven distribution. */
 *hit=chance*100;
 return !eval_bad;
}
