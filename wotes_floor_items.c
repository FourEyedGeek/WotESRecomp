#include "wotes_floor_items.h"
#include "genesis_runtime.h"
#include "video/genesis_machine.h"
#include <stdlib.h>
#include <string.h>

/* Host-generated art occupies the unused ROM shadow above the supplied
 * 2 MB cartridge, outside its SRAM window. No cartridge file is changed.
 * The original renderer uploads four near tiles, one middle tile and one
 * far tile. Preserve that layout and its normal depth/occlusion handling. */
enum { GAUNTLETS=0x81, FLOOR_ART=0x300000 };
static int s_ready;
extern GenesisMachine g_machine;
static unsigned pixel(const uint8_t *tiles, unsigned x, unsigned y) {
    unsigned tile=(x/8)*2+y/8;
    uint8_t b=tiles[tile*32+(y&7)*4+((x&7)/2)];
    return (b>>(x&1?0:4))&15u;
}
static void small_icon(uint8_t *out,const uint8_t *source,unsigned size) {
    memset(out,0,32);
    for (unsigned y=0;y<size;++y) for(unsigned x=0;x<size;++x) {
        /* Retain nontransparent detail in each source area rather than
         * losing thin glove fingers to a single nearest-neighbour sample. */
        unsigned best=0;
        for(unsigned sy=y*16/size;sy<(y+1)*16/size;++sy)
            for(unsigned sx=x*16/size;sx<(x+1)*16/size;++sx) {
                unsigned n=pixel(source,sx,sy);
                if(n>best)best=n;
            }
        unsigned dx=x+(8-size)/2,dy=y+8-size;
        out[dy*4+dx/2]|=(uint8_t)(best<<(dx&1?0:4));
    }
}
void wotes_floor_items_init(void) {
    unsigned record=0x3E58Cu+(GAUNTLETS-1)*16;
    unsigned art=0xB636Eu+(unsigned)g_rom[record+2]*128;
    /* $8372 loads this same uncompressed, column-major inventory artwork.
     * The gauntlets have no inventory recolour ($FF at record+3). */
    memcpy(g_rom+FLOOR_ART,g_rom+art,128);
    small_icon(g_rom+FLOOR_ART+128,g_rom+art,8);
    small_icon(g_rom+FLOOR_ART+160,g_rom+art,6);
    s_ready=1;
}
void wotes_floor_items_select(void) {
    const char *mode=getenv("WOTES_FLOOR_ITEM_ICONS");
    if(mode && !strcmp(mode,"0"))return;
    /* $1FB60 follows the shared floor-art lookup. A5 selects one of the
     * 27 visible cells; $DF93 contains that cell's actual item identifier.
     * Redirect only the gauntlets, before the original distance offsets. */
    unsigned cell=g_cpu.A[5];
    if(cell<27 && g_ram[0xDF93+cell]==GAUNTLETS) {
        if(!s_ready)wotes_floor_items_init();
        g_cpu.A[1]=FLOOR_ART;
        g_cpu.D[4]=0;
    }
}

void wotes_floor_item_pickup(int finished) {
    const char *mode=getenv("WOTES_FLOOR_ITEM_ICONS");
    if(mode && !strcmp(mode,"0"))return;
    /* $21320's pickup flight retains the actual item ID in D2. Its
     * gauntlets descriptor ($1EC42) uses the shared four-tile icon at
     * VRAM $9160 (tile $48B). Swap that artwork during this flight;
     * restore the original $1EA4C upload afterwards for other magic items.
     * Neither the flight movement nor the inventory transfer is replaced. */
    if((g_cpu.D[2]&0xFFFFu)!=GAUNTLETS)return;
    if(!s_ready)wotes_floor_items_init();
    memcpy(g_machine.vdp.vram+0x9160,
           g_rom+(finished?0xFFF70u:FLOOR_ART),128);
}
