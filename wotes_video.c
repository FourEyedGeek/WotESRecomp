#include "wotes_video.h"
#include "wotes_automap.h"
#include "wotes_shop.h"
#include "genesis_runtime.h"
#include "video/genesis_machine.h"
#include "video/genesis_dac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { ORIGINAL_WIDTH=320, ORIGINAL_MAP=224, UI_WIDTH=96, WIDE_WIDTH=398 };
static int s_enabled=1, s_scene, s_dialogue_y=-1;
static int s_dialogue_end=224, s_dialogue_left=8;
static int s_camera[2][2];

static unsigned be16(const uint8_t *p) { return ((unsigned)p[0]<<8)|p[1]; }
static unsigned ram16(unsigned a) { return be16(g_ram+(a&65535)); }
static unsigned ram32(unsigned a) { return (ram16(a)<<16)|ram16(a+2); }
static unsigned rom16(unsigned a) { return a+1<0x100000u?be16(g_rom+a):0; }
static int wrap(int x,int n) { x%=n;return x<0?x+n:x; }

/* $22A98 builds small top-of-screen messages, including the combat gold
 * reward. Unlike bottom dialogue, its left frame is at tile column one. */
int wotes_video_reward_window(const GVDP *v)
{
    unsigned row=v->reg[18]&31,base=(v->reg[3]&62)*1024;
    return !(v->reg[18]&128) && row>=3 && row<=12 &&
        be16(v->vram+((base+2)&65535))==0x85A9 &&
        be16(v->vram+((base+54)&65535))==0x85AA &&
        be16(v->vram+((base+(row-1)*128+2)&65535))==0x85AB &&
        be16(v->vram+((base+(row-1)*128+54)&65535))==0x85AC;
}

/* Audited from $BBEA/$BD30: each 32x32 layout selects a 192px block;
 * its 8x8 metatile dictionary selects 24px (3x3 tile) graphics records.
 * The two map descriptors are at $FF8648/$FF8736. No guest writes. */
static unsigned map_attr(int plane,int tx,int ty)
{
    unsigned descriptor=plane?0x8736:0x8648;
    unsigned layout=ram32(descriptor+24), blocks=ram32(0x8640), tiles=ram32(0x8644);
    tx=wrap(tx,768);ty=wrap(ty,768);
    unsigned block=rom16(layout+((ty/24)*32+tx/24)*2);
    unsigned meta=rom16(blocks+block*128+((ty%24)/3)*16+((tx%24)/3)*2);
    unsigned attr=rom16(tiles+meta*18+(ty%3)*6+(tx%3)*2);
    return plane?attr&0x7FFFu:attr;
}

/* The native DMA can trail the simulation's camera within the same frame.
 * Match its ring phase, then require the decoded map to agree at 32 points.
 * Camp, caves, story and unknown layouts fail closed to the centered image. */
static int qualify(const GVDP *v)
{
    s_dialogue_y=-1;
    s_dialogue_end=224;s_dialogue_left=8;
    if(!(v->reg[1]&0x40) || !(v->reg[12]&1) || (v->reg[12]&14) ||
       v->reg[17]!=0x8E || v->reg[16]!=1 || (v->reg[11]&7))return 0;
    if(v->reg[18]) {
        /* Dialogue enables the bottom VDP window over the unchanged map
         * planes. Recognize its actual frame, rather than admitting menus
         * merely because their map descriptors still contain old data. */
        unsigned row=v->reg[18]&31,base=(v->reg[3]&62)*1024;
        if(wotes_video_reward_window(v)) {
            s_dialogue_y=0;s_dialogue_end=(int)row*8;s_dialogue_left=16;
        } else {
        if(!(v->reg[18]&128) || !row || row>=27 ||
           be16(v->vram+((base+row*128)&65535))!=0x85A9 ||
           be16(v->vram+((base+row*128+54)&65535))!=0x85AA ||
           be16(v->vram+((base+27*128)&65535))!=0x85AB ||
           be16(v->vram+((base+27*128+54)&65535))!=0x85AC)return 0;
        s_dialogue_y=(int)row*8;
        }
    }
    unsigned blocks=ram32(0x8640),tiles=ram32(0x8644);
    if(blocks<0x200 || blocks>=0x100000 || tiles<0x200 || tiles>=0x100000)return 0;
    unsigned hbase=(v->reg[13]&63)*1024;
    for(int p=0;p<2;++p) {
        unsigned d=p?0x8736:0x8648, layout=ram32(d+24);
        if(layout<0x200 || layout+2048>0x100000)return 0;
        int x=ram16(d),y=ram16(d+2);
        int hx=(-((int)be16(v->vram+((hbase+p*2)&65535))))&511;
        int vy=v->vsram[p]&255;
        x+=wrap(hx-(x&511)+256,512)-256;
        y+=wrap(vy-(y&255)+128,256)-128;
        s_camera[p][0]=x;s_camera[p][1]=y;
        unsigned base=p?(v->reg[4]&7)*8192:(v->reg[2]&56)*1024;
        for(int sy=16;sy<=208;sy+=64)for(int sx=16;sx<=208;sx+=64) {
            int tx=(x+sx)/8,ty=(y+sy)/8;
            unsigned addr=(base+((ty&31)*64+(tx&63))*2)&65535;
            if(be16(v->vram+addr)!=map_attr(p,tx,ty))return 0;
        }
    }
    return 1;
}

static unsigned pattern(const GVDP *v,unsigned attr,int x,int y)
{
    x&=7;y&=7;
    if(attr&0x800)x^=7;
    if(attr&0x1000)y^=7;
    unsigned addr=((attr&2047)*32+y*4+(x>>1))&65535;
    return (v->vram[addr]>>(x&1?0:4))&15;
}

static uint32_t terrain(const GVDP *v,int x,int y,unsigned *high)
{
    unsigned color=v->reg[7]&63;*high=0;
    for(int p=1;p>=0;--p) {
        int wx=s_camera[p][0]+x,wy=s_camera[p][1]+y;
        int tx=wx>=0?wx/8:(wx-7)/8,ty=wy>=0?wy/8:(wy-7)/8;
        unsigned attr=map_attr(p,tx,ty),n=pattern(v,attr,wx,wy);
        if(n && ((attr&0x8000) || !*high)) {
            color=((attr>>13)&3)*16+n;*high=(attr>>15)&1;
        }
    }
    return genesis_dac_cram_to_argb(v->cram[color],GENESIS_DAC_NORMAL);
}

/* Read-only SAT extension. Entry zero is the fixed HUD selector ($1E3A).
 * All world entries retain native link order, pattern layout and priority.
 * This does not broaden simulation activation or change sprite status bits. */
static void edge_sprites(const GVDP *v,int line,uint32_t *out,int map_width,int left,
                         const unsigned *plane_high)
{
    unsigned sat=(v->reg[5]&126)*512,link=0;
    uint8_t occupied[WIDE_WIDTH]={0};
    for(unsigned i=0;i<80;++i) {
        unsigned a=(sat+link*8)&65535;
        unsigned sy=be16(v->vram+a)&511,size=be16(v->vram+((a+2)&65535));
        unsigned attr=be16(v->vram+((a+4)&65535)),sx=be16(v->vram+((a+6)&65535))&511;
        int h=((size>>8)&3)+1,w=((size>>10)&3)+1,ry=line-((int)sy-128);
        if(i && ry>=0 && ry<h*8 && sx) {
            int yy=(attr&0x1000)?h*8-1-ry:ry;
            for(int xx=0;xx<w*8;++xx) {
                int ox=(int)sx-128+xx+left;
                if(ox<0 || ox>=map_width || (ox>=left && ox<left+ORIGINAL_MAP) || occupied[ox])continue;
                int px=(attr&0x800)?w*8-1-xx:xx;
                unsigned tile=((attr&2047)+(px/8)*h+yy/8)&2047;
                unsigned n=pattern(v,tile,px&7,yy&7);
                if(!n)continue;
                occupied[ox]=1;
                if(!(attr&0x8000) && plane_high[ox])continue;
                out[ox]=genesis_dac_cram_to_argb(v->cram[((attr>>13)&3)*16+n],GENESIS_DAC_NORMAL);
            }
        }
        link=size&127;if(!link || link>=80)break;
    }
}

static int configure(const char *mode)
{
    if(!mode)return 0;
    if(!strcmp(mode,"off")){s_enabled=0;return 1;}
    if(strcmp(mode,"16:9") && strcmp(mode,"fit") && strcmp(mode,"stage"))return 0;
    s_enabled=1;return 1;
}
static int enabled(void) { return s_enabled; }
static int width(int dw,int dh,int nw,int nh)
{ (void)dw;(void)dh;(void)nw;(void)nh;return s_enabled?WIDE_WIDTH:0; }

static void scanline(const GVDP *v,int line,const uint32_t *native,int native_width,
                     uint32_t *out,int canvas)
{
    for(int x=0;x<canvas;++x)out[x]=0xFF000000u;
    if(line==0)s_scene=(wotes_shop_scene(v)||wotes_inventory_scene(v))?3:wotes_automap_scene(v)?2:qualify(v);
    if(native_width==320 && canvas==WIDE_WIDTH && s_scene==3) {
        wotes_shop_row(line,native,out);
        return;
    }
    if(native_width==320 && canvas==WIDE_WIDTH && s_scene==2) {
        wotes_automap_row(line,native,out);
        return;
    }
    if(native_width!=320 || canvas!=WIDE_WIDTH || !s_scene) {
        int left=(canvas-native_width)/2;
        if(left>=0)memcpy(out+left,native,native_width*sizeof *out);
        return;
    }
    int map_width=canvas-UI_WIDTH,left=(map_width-ORIGINAL_MAP)/2;
    unsigned priority[WIDE_WIDTH]={0};
    for(int x=0;x<map_width;++x) {
        if(x>=left && x<left+ORIGINAL_MAP)out[x]=native[x-left];
        else out[x]=terrain(v,x-left,line,&priority[x]);
    }
    edge_sprites(v,line,out,map_width,left,priority);
    if(s_dialogue_y>=0 && line>=s_dialogue_y && line<s_dialogue_end) {
        /* Keep every original text pixel and its line breaks. Extend the
         * frame on both sides using its horizontal rail or blank font tile;
         * corners and side rails move to the wider map's outside edges. */
        unsigned attr=(line<s_dialogue_y+8 || line>=s_dialogue_end-8)?0x85A7:0x85B1;
        for(int x=s_dialogue_left;x<map_width-8;++x) {
            unsigned n=pattern(v,attr,x,line);
            out[x]=genesis_dac_cram_to_argb(v->cram[((attr>>13)&3)*16+n],GENESIS_DAC_NORMAL);
        }
        memcpy(out,native,s_dialogue_left*sizeof *out);
        memcpy(out+map_width-8,native+ORIGINAL_MAP-8,8*sizeof *out);
        memcpy(out+left+s_dialogue_left,native+s_dialogue_left,
               (ORIGINAL_MAP-8-s_dialogue_left)*sizeof *out);
    }
    memcpy(out+map_width,native+ORIGINAL_MAP,UI_WIDTH*sizeof *out);
}

const GameVideo wotes_video={
    .configure=configure,.enabled=enabled,.width=width,.scanline=scanline,
};

/* Explicit development captures only; the normal build does no file I/O here. */
void wotes_video_probe(uint64_t frame)
{
    const char *trace=getenv("WOTES_VIDEO_TRACE");
    if(trace && *trace) {
        static FILE *log;
        if(!log){log=fopen(trace,"w");if(log)fputs("frame,expanded,camera_x,camera_y\n",log);}
        if(log){fprintf(log,"%llu,%d,%d,%d\n",(unsigned long long)frame,s_scene,s_camera[0][0],s_camera[0][1]);fflush(log);}
    }
    const char *dir=getenv("WOTES_CAPTURE_DIR");
    if(!dir || !*dir || (frame!=1496 && frame!=1586))return;
    char path[1024];
    snprintf(path,sizeof path,"%s/scene_%llu.bin",dir,(unsigned long long)(frame+1));
    FILE *f=fopen(path,"wb");if(!f)return;
    fwrite(g_ram,1,sizeof g_ram,f);
    fwrite(g_machine.vdp.vram,1,sizeof g_machine.vdp.vram,f);
    fwrite(g_machine.vdp.cram,2,64,f);
    fwrite(g_machine.vdp.vsram,2,40,f);
    fwrite(g_machine.vdp.reg,1,24,f);
    fclose(f);
}
