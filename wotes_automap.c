#include "wotes_automap.h"
#include "wotes_video.h"
#include "genesis_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

/* Host-only survey. A visited cell and its traversed edges are recorded;
 * no hidden dungeon geometry is read or disclosed. $DC5E/$DC60 are the
 * original dungeon coordinates, eight units per cell. $DC62 is N/E/S/W.
 * $E22E/$E230 are the map dimensions used by $10B54. $C600/$C622 identify
 * the loaded dungeon/layout. The stable build does not compile this module. */
enum { LIMIT=128, MAPS=64, VISITED=16, START=32, PANEL_X=0, PANEL_W=78, UI_X=302 };
typedef struct Survey {
    uint32_t id, layout;
    uint16_t width, height;
    uint8_t cells[LIMIT*LIMIT];
} Survey;
static Survey s_maps[MAPS];
static uint8_t s_has_start[MAPS];
static unsigned s_count;
static int s_current=-1, s_x, s_y, s_facing, s_previous=-1, s_px, s_py;
static char s_path[1024];
static unsigned r16(unsigned a) { return ((unsigned)g_ram[a]<<8)|g_ram[a+1]; }
static unsigned r32(unsigned a) { return (r16(a)<<16)|r16(a+2); }

void wotes_automap_reset_position(void) { s_previous=-1; }

static void persist(void)
{
    if(!*s_path)return;
    char temp[1060];snprintf(temp,sizeof temp,"%s.tmp",s_path);
    FILE *f=fopen(temp,"wb");if(!f)return;
    const uint32_t header[]={0x50414D57u,1u,WOTES_ROM_CRC,s_count};
    int ok=fwrite(header,sizeof header,1,f)==1 && fwrite(s_maps,sizeof(Survey),s_count,f)==s_count;
    if(fclose(f))ok=0;
    if(!ok){remove(temp);return;}
#ifdef _WIN32
    if(!MoveFileExA(temp,s_path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))remove(temp);
#else
    if(rename(temp,s_path))remove(temp);
#endif
}

void wotes_automap_load(const char *path,const uint8_t *data,size_t size,int loaded)
{
    (void)data;(void)size;(void)loaded;
    s_count=0;s_current=-1;s_previous=-1;memset(s_maps,0,sizeof s_maps);memset(s_has_start,0,sizeof s_has_start);
    snprintf(s_path,sizeof s_path,"%s.automap",path);
    FILE *f=fopen(s_path,"rb");if(!f)return;
    uint32_t h[4];
    if(fread(h,sizeof h,1,f)==1 && h[0]==0x50414D57u && h[1]==1 && (h[2]==WOTES_ROM_CRC || h[2]==0xAF4A9CD7u) && h[3]<=MAPS &&
       fread(s_maps,sizeof(Survey),h[3],f)==h[3]) {
        s_count=h[3];
        for(unsigned i=0;i<s_count;++i)
            if(!s_maps[i].width || s_maps[i].width>LIMIT || !s_maps[i].height || s_maps[i].height>LIMIT){s_count=0;break;}
    }
    fclose(f);
    for(unsigned i=0;i<s_count;++i)
        for(unsigned n=0;n<LIMIT*LIMIT;++n)if(s_maps[i].cells[n]&START){s_has_start[i]=1;break;}
}

static int position(unsigned *id,unsigned *layout,int *w,int *h,int *x,int *y)
{
    if(g_ram[0xA93B]!=255 || g_ram[0x8005]!=0xFC)return 0;
    *w=r16(0xE22E);*h=r16(0xE230);*x=r16(0xDC5E)/8;*y=r16(0xDC60)/8;
    *id=r16(0xC600);*layout=r32(0xC622);
    return *w>0 && *w<=LIMIT && *h>0 && *h<=LIMIT && *x<*w && *y<*h &&
           *layout>=0x200 && *layout<0x100000 && r16(0xDC62)<4;
}

void wotes_automap_tick(uint64_t frame)
{
    (void)frame;
    unsigned id,layout;int w,h,x,y;
    if(!position(&id,&layout,&w,&h,&x,&y)){s_previous=-1;return;}
    int current=-1;
    for(unsigned i=0;i<s_count;++i)
        if(s_maps[i].id==id && s_maps[i].layout==layout && s_maps[i].width==w && s_maps[i].height==h){current=(int)i;break;}
    if(current<0){
        if(s_count==MAPS){s_current=-1;return;}
        current=(int)s_count++;Survey *m=&s_maps[current];memset(m,0,sizeof *m);
        m->id=id;m->layout=layout;m->width=(uint16_t)w;m->height=(uint16_t)h;
    }
    Survey *m=&s_maps[current];unsigned cell=y*LIMIT+x;
    int dirty=!(m->cells[cell]&VISITED);m->cells[cell]|=VISITED;
    /* The spare cell bit preserves the existing survey file format. Old
     * surveys retain all discoveries and mark the first entry after upgrade. */
    if(!s_has_start[current]){m->cells[cell]|=START;s_has_start[current]=1;dirty=1;}
    if(s_previous==current){
        int dx=x-s_px,dy=y-s_py,dir=-1;
        if(dx==0 && dy==-1)dir=0;else if(dx==1 && dy==0)dir=1;
        else if(dx==0 && dy==1)dir=2;else if(dx==-1 && dy==0)dir=3;
        if(dir>=0){
            unsigned previous=s_py*LIMIT+s_px;
            if(!(m->cells[previous]&(1<<dir)))dirty=1;
            m->cells[previous]|=(uint8_t)(1<<dir);m->cells[cell]|=(uint8_t)(1<<((dir+2)&3));
        }
    }
    s_current=current;s_x=x;s_y=y;s_facing=(int)r16(0xDC62);
    const char *trace=getenv("WOTES_AUTOMAP_TRACE");
    if(trace && *trace) {
        static FILE *log;
        if(!log){log=fopen(trace,"w");if(log)fputs("frame,id,layout,x,y,facing\n",log);}
        if(log){fprintf(log,"%llu,%u,%u,%d,%d,%d\n",(unsigned long long)frame,id,layout,x,y,s_facing);fflush(log);}
    }
    s_previous=current;s_px=x;s_py=y;
    if(dirty)persist();
}

int wotes_automap_scene(const GVDP *v)
{
    unsigned id,layout;int w,h,x,y;
    return position(&id,&layout,&w,&h,&x,&y) && (v->reg[1]&0x40) &&
           (v->reg[12]&1) && v->reg[17]==0x8E &&
           (v->reg[18]==0 || wotes_video_reward_window(v));
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
    default:return "000000000000000";
    }
}

static void label_row(uint32_t *out,int line,int x,int y,const char *text,int scale,uint32_t color)
{
    int row=(line-y)/scale;if(line<y || row>=5)return;
    for(;*text;++text,x+=4*scale){
        const char *g=glyph(*text);
        for(int c=0;c<3;++c)if(g[row*3+c]=='1')
            for(int n=0;n<scale;++n)if(x+c*scale+n>=PANEL_X && x+c*scale+n<PANEL_X+PANEL_W)out[x+c*scale+n]=color;
    }
}

void wotes_automap_row(int line,const uint32_t *native,uint32_t *out)
{
    const uint32_t bg=0xFF101C29u,gold=0xFFD3A94Fu,ink=0xFF69949Cu,trail=0xFF83CDD2u,player=0xFFFFDF6Eu,start=0xFF65D88Au;
    memcpy(out+PANEL_W,native,224*sizeof *out);memcpy(out+UI_X,native+224,96*sizeof *out);
    for(int x=PANEL_X;x<PANEL_X+PANEL_W;++x)out[x]=bg;
    out[0]=out[77]=gold;
    if(line==0 || line==223 || line==23 || line==197)for(int x=0;x<PANEL_W;++x)out[x]=gold;
    label_row(out,line,21,7,"MAP",2,gold);
    label_row(out,line,10,184,"N",1,ink);
    label_row(out,line,23,204,"TRAIL",1,trail);
    if(s_current<0)return;
    Survey *m=&s_maps[s_current];
    char coords[24];snprintf(coords,sizeof coords,"X%d Y%d",s_x,s_y);
    label_row(out,line,6,214,coords,1,ink);
    enum { CELL=8, COLS=8, ROWS=18, ORIGIN_X=7, ORIGIN_Y=31 };
    int start_x=m->width<=COLS?0:s_x-COLS/2,start_y=m->height<=ROWS?0:s_y-ROWS/2;
    if(start_x<0)start_x=0;if(start_y<0)start_y=0;
    if(m->width>COLS && start_x>m->width-COLS)start_x=m->width-COLS;
    if(m->height>ROWS && start_y>m->height-ROWS)start_y=m->height-ROWS;
    int py=line-ORIGIN_Y;if(py<0 || py>=ROWS*CELL)return;
    int y=start_y+py/CELL,ry=py%CELL;if(y>=m->height)return;
    for(int col=0;col<COLS;++col){
        int x=start_x+col;if(x>=m->width)break;
        unsigned cell=m->cells[y*LIMIT+x];int ox=ORIGIN_X+col*CELL;
        if(cell&VISITED){
            if(ry>=2 && ry<=5)for(int n=2;n<=5;++n)out[ox+n]=(cell&START)?start:trail;
            if((cell&1) && ry<2)out[ox+3]=out[ox+4]=trail;
            if((cell&2) && (ry==3 || ry==4))out[ox+6]=out[ox+7]=trail;
            if((cell&4) && ry>=6)out[ox+3]=out[ox+4]=trail;
            if((cell&8) && (ry==3 || ry==4))out[ox]=out[ox+1]=trail;
        }
        if(x==s_x && y==s_y){
            if(ry>=2 && ry<=5)for(int n=2;n<=5;++n)out[ox+n]=(cell&START)?start:player;
            if(s_facing==0 && ry<2)out[ox+3]=out[ox+4]=player;
            if(s_facing==1 && (ry==3 || ry==4))out[ox+6]=out[ox+7]=player;
            if(s_facing==2 && ry>=6)out[ox+3]=out[ox+4]=player;
            if(s_facing==3 && (ry==3 || ry==4))out[ox]=out[ox+1]=player;
        }
    }
}
