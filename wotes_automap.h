#pragma once
#include <stddef.h>
#include <stdint.h>
#include "video/genesis_machine.h"
void wotes_automap_load(const char *, const uint8_t *, size_t, int);
void wotes_automap_tick(uint64_t);
void wotes_automap_reset_position(void);
int wotes_automap_scene(const GVDP *);
void wotes_automap_row(int, const uint32_t *, uint32_t *);
