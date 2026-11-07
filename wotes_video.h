#pragma once
#include <stdint.h>
#include "game_video.h"
extern const GameVideo wotes_video;
void wotes_video_probe(uint64_t frame);
struct GVDP;
int wotes_video_reward_window(const struct GVDP *v);
