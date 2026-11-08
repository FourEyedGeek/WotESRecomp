#pragma once
#include <stdint.h>
#include "video/genesis_vdp.h"
int wotes_shop_scene(const GVDP *v);
int wotes_inventory_scene(const GVDP *v);
void wotes_shop_row(int line,const uint32_t *native,uint32_t *out);
int wotes_shop_ac(const uint8_t *ram,unsigned actor,unsigned item,int *out);
/* Nominal d20 chance, in hundredths of one percent, with current effects.
 * Status marks stored timed effects; preparation rolls are not averaged. */
int wotes_shop_weapon(const uint8_t *ram,unsigned actor,unsigned item,int *low,int *high,int *hit,int *status);
