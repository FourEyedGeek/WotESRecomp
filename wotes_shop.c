#include "wotes_shop.h"
#include "genesis_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char rows[4][6][20];
static uint32_t colors[4][6];
static unsigned u16(const uint8_t *p){return (p[0]<<8)|p[1];}
static unsigned rec(unsigned id){return 0x3e58c+(id-1)*16;}
static int text_at(const GVDP *v,unsigned base,int row,int col,const char *s){
 for(;*s;++s,++col)if((u16(v->vram+((base+row*128+col*2)&65535))&2047)!=(unsigned char)*s+0x591)return 0;
 return 1;
}
static int allowed(unsigned actor,unsigned id){
 unsigned cls=g_ram[actor]&7;
 unsigned kind=id?g_rom[rec(id)]:0;
 /* Armour is worn automatically from carried items ($13810); the action
  * assignment mask below applies to weapons, not armour. */
 if(kind==4)return cls!=3 && (cls!=4 || id<107);
 if(kind==5)return cls!=3 && cls!=4;
 return id && (g_rom[0x3f537+id-1]&(1u<<cls));
}
static void refresh(unsigned item){
 const uint32_t ink=0xffc8d7df,gold=0xffe0b962,green=0xff79d9a1,red=0xfff09b90;
 unsigned kind=item?g_rom[rec(item)]:0;
 for(int i=0;i<4;++i){
  unsigned a=0x912c+150*i;memset(rows[i],0,sizeof rows[i]);
  for(int n=0;n<6;++n)colors[i][n]=ink;
  snprintf(rows[i][0],20,"%.17s",(char*)g_ram+a+2);colors[i][0]=gold;
  if(!item){strcpy(rows[i][2],"HIGHLIGHT AN ITEM");continue;}
  int now=0,next=0,can=allowed(a,item);
  if(kind==4 || kind==5){
   if(wotes_shop_ac(g_ram,a,0,&now))snprintf(rows[i][2],20,"CURRENT AC %d",now);
   else strcpy(rows[i][2],"CURRENT AC ?");
   if(!can){strcpy(rows[i][3],"CANNOT WEAR");colors[i][3]=red;}
   else if(wotes_shop_ac(g_ram,a,item,&next)){
    snprintf(rows[i][3],20,"WITH ITEM  %d",next);colors[i][3]=next<now?green:next>now?red:ink;
   }else strcpy(rows[i][3],"WITH ITEM  ?");
   strcpy(rows[i][5],"LOWER AC IS BETTER");
  }else if(kind==2){
   int lo,hi,hit;unsigned current=g_ram[a+30];
   strcpy(rows[i][1],"READIED A / NEW");
   if(!current || g_rom[rec(current)]!=2){current=g_ram[a+31];strcpy(rows[i][1],"READIED B / NEW");}
   int ok=wotes_shop_weapon(g_ram,a,current,&lo,&hi,&hit);
   if(ok){snprintf(rows[i][2],20,"DMG %d-%d",lo,hi);snprintf(rows[i][4],20,"HIT %d%%",hit);}
   else{strcpy(rows[i][2],"DMG SPECIAL");strcpy(rows[i][4],"HIT SPECIAL");}
   int old_sum=ok?lo+hi:0,old_hit=ok?hit:0;
   if(!can){strcpy(rows[i][3],"CANNOT USE");colors[i][3]=red;}
   else if(wotes_shop_weapon(g_ram,a,item,&lo,&hi,&hit)){
    snprintf(rows[i][3],20," -> %d-%d",lo,hi);snprintf(rows[i][5],20," -> %d%% VS AC 0",hit);
    colors[i][3]=!ok || lo+hi==old_sum?ink:lo+hi>old_sum?green:red;
    colors[i][5]=!ok || hit==old_hit?ink:hit>old_hit?green:red;
   }else{strcpy(rows[i][3],"ENEMY DEPENDENT");strcpy(rows[i][5],"SPECIAL EFFECT");}
  }else strcpy(rows[i][2],"NO EQUIPMENT STATS");
 }
 const char *trace=getenv("WOTES_SHOP_TRACE");
 if(trace && *trace){FILE *f=fopen(trace,"a");if(f){fprintf(f,"item=%u kind=%u\n",item,kind);for(int i=0;i<4;++i){for(int j=0;j<6;++j)fprintf(f,"%s | ",rows[i][j]);fputc('\n',f);}fclose(f);}}
}
int wotes_shop_scene(const GVDP *v){
 if(!(v->reg[1]&0x40) || !(v->reg[12]&1) || v->reg[17]!=0x8e)return 0;
 unsigned base=(v->reg[2]&56)*1024;int found=0;
 for(int x=1;x<=23;++x)if(text_at(v,base,1,x," SHOP"))found=1;
 if(!found || !text_at(v,base,7,5,"BUY"))return 0;
 int x=(int)u16(g_ram+0xa3b4)-144,y=(int)u16(g_ram+0xa3b6)-232;
 unsigned item=0;
 if(x>=0 && x%24==0 && x/24<8 && y>=0 && y%24==0 && y/24<4){
  unsigned index=y/24*8+x/24;item=g_ram[0x1000+index];
 }
 static uint8_t previous[600];static unsigned last=~0u;
 if(item!=last || memcmp(previous,g_ram+0x912c,600)){
  memcpy(previous,g_ram+0x912c,600);last=item;refresh(item);
 }
 return 1;
}
static const char *glyph(char c)
{
    switch(c){
    case 'M':return "101111111101101";case 'A':return "010101111101101";
    case 'P':return "110101110100100";case 'N':return "101111111111101";
    case 'E':return "111100110100111";case 'S':return "111100111001111";
    case 'W':return "101101111111101";case 'T':return "111010010010010";
    case 'R':return "110101110101101";case 'I':return "111010010010111";
    case 'L':return "100100100100111";case 'X':return "101101010101101";
    case 'Y':return "101101010010010";case 'D':return "110101101101110";
    case '0':return "111101101101111";case '1':return "010110010010111";
    case '2':return "110001010100111";case '3':return "110001010001110";
    case '4':return "101101111001001";case '5':return "111100110001110";
    case '6':return "011100111101111";case '7':return "111001010010010";
    case '8':return "111101111101111";case '9':return "111101111001110";
    case 'B':return "110101110101110";
    case 'C':return "111100100100111";
    case 'F':return "111100110100100";
    case 'G':return "111100101101111";
    case 'H':return "101101111101101";
    case 'J':return "001001001101111";
    case 'K':return "101101110101101";
    case 'O':return "111101101101111";
    case 'Q':return "111101101111001";
    case 'U':return "101101101101111";
    case 'V':return "101101101101010";
    case 'Z':return "111001010100111";
    case '-':return "000000111000000";
    case '>':return "100010001010100";
    case '+':return "000010111010000";
    case '%':return "101001010100101";
    case ':':return "000010000010000";
    case '/':return "001001010100100";
    case '?':return "110001010000010";
    default:return "000000000000000";
    }
}


void wotes_shop_row(int line,const uint32_t *native,uint32_t *out){
 memcpy(out,native,320*sizeof *out);
 for(int x=320;x<398;++x)out[x]=0xff101c29;
 out[320]=out[397]=0xffd3a94f;
 int i=line/56,local=line%56;
 if(!local || line==223){for(int x=320;x<398;++x)out[x]=0xffd3a94f;return;}
 if(i>=4)return;
 for(int n=0;n<6;++n){
  int y=local-(5+n*8);if(y<0 || y>=5)continue;
  int x=325;
  for(const char *s=rows[i][n];*s;++s,x+=4){const char *g=glyph(*s);
   for(int k=0;k<3;++k)if(x+k<397 && g[y*3+k]=='1')out[x+k]=colors[i][n];
  }
 }
}
