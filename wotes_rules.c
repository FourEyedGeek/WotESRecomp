#include "wotes_rules.h"
static int party_actor(unsigned a){return a>=0x912c && a<=0x92ee && (a-0x912c)%150==0;}
int wotes_stench_active(const uint8_t *ram,unsigned actor){
 if(!party_actor(actor))return 0;
 for(unsigned k=92;k<102;k+=2)if(ram[actor+k]==0xd6 && ram[actor+k+1])return 1;
 return 0;
}
