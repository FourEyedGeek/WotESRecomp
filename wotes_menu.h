#ifndef WOTES_MENU_H
#define WOTES_MENU_H
#include <SDL2/SDL.h>
enum { WOTES_MENU_RESUME=0, WOTES_MENU_RESET=1, WOTES_MENU_EXIT=2,
       WOTES_MENU_SAVE_BASE=10, WOTES_QUICK_SLOT=5 };
void wotes_menu_init(const char *settings_path);
void wotes_menu_apply(void);
int wotes_menu_intro_enabled(void);
int wotes_menu_run(SDL_Renderer *renderer, SDL_Window *window, int playing);
void wotes_menu_fullscreen_changed(SDL_Window *window);
void wotes_menu_state_path(int slot,char *out,size_t cap);
/* Scheduler-owned state I/O; called from the paused host menu or hotkeys. */
int wotes_runner_save_slot(int slot);
int wotes_runner_load_slot(int slot);
/* Explicit local test hooks; inactive without WOTES_MENU_TEST_SCRIPT. */
void wotes_menu_test_device(void);
void wotes_menu_test_tick(int in_menu);
#endif
