#include "game_spec.h"
#include "genesis_runtime.h"
#include "video/genesis_machine.h"
#include "wotes_video.h"
#include "wotes_automap.h"
#include "wotes_floor_items.h"
#include "wotes_menu.h"
#include "glue.h"
#include "m68k_interp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern GenesisMachine g_machine;

/* This patched 2 MB cartridge's SRAM is at $200001-$20FFFF, outside the ROM.
 * It is readable/writable at power-on; it does not require the game to
 * enable an overlapping-ROM bank. The shared bus currently starts gated. */
static void cartridge_reset(void) {
    g_machine.bus.sram_enabled = 1;
    wotes_floor_items_init();
}

/* Skip the publisher presentation and report the original intro's
 * user-dismissed result (carry set). $166C6 then passes D0.b=0 to $16EA8,
 * retaining its complete title/menu setup while omitting the animation.
 * No save data, menu selection, or new-game story is bypassed. */
static uint8_t s_inscription_buttons;

/* File states contain guest addresses, never process-specific C stack pointers.
 * Resume the interrupted caller through the existing instruction floor until
 * reaching another compiled entry, then continue using native routines. */
typedef struct WotesState { uint32_t pc, sp; uint8_t inscription; } WotesState;
static WotesState s_resume;
extern int game_dispatch_table_size(void);
extern uint32_t game_dispatch_table_addr(int i);
static int native_entry(uint32_t pc) {
    /* These sound helpers share the return-capture pattern already kept on
     * the instruction floor by game.toml. A resumed interior caller must
     * retain that policy for its nested helpers too. */
    if (pc>=0x1ED00u && pc<0x1F000u) return 0;
    int lo=0,hi=game_dispatch_table_size();
    while (lo<hi) { int mid=lo+(hi-lo)/2;
        if (game_dispatch_table_addr(mid)<pc) lo=mid+1; else hi=mid; }
    return lo<game_dispatch_table_size() && game_dispatch_table_addr(lo)==pc;
}
extern void recomp_tail_frame_set(void *frame);
extern int g_split_sp_popped;
extern int genesis_force_interp(void);
static size_t state_size(void) { return sizeof(WotesState); }
static int state_save(void *data, size_t size) {
    if (size != sizeof(WotesState)) return 0;
    WotesState state = {0};
    state.pc = g_cpu.PC; state.sp = g_cpu.A[7];
    state.inscription = s_inscription_buttons;
    memcpy(data, &state, size); return 1;
}
static int state_load(const void *data, size_t size, int apply) {
    WotesState state;
    if (size != sizeof state) return 0;
    memcpy(&state, data, size);
    if ((state.pc != 0xBA70u && state.pc != 0xBA80u) ||
        (state.sp & 1) || (state.sp & 0xFFFFFFu) < 0xFF8000u ||
        (state.sp & 0xFFFFFFu) > 0xFFFFF6u) return 0;
    if (apply) { s_resume = state; s_inscription_buttons = state.inscription; }
    return 1;
}
static int state_boundary(void) { return 1; }
static void resume_state(uint32_t unused) {
    (void)unused;
    g_cpu.PC = s_resume.pc; g_cpu.A[7] = s_resume.sp;
    recomp_tail_frame_set(NULL); g_split_sp_popped = 0; g_rte_pending = 0;
    for (;;) {
        uint32_t pc = g_cpu.PC & 0xFFFFFFu;
        if (native_entry(pc) && !genesis_force_interp()) {
            g_split_sp_popped = 0;
            recomp_call_addr(pc);
            /* Native RTS leaves the guest return for its generated caller. */
            g_rte_pending = 0;
            g_cpu.PC = m68k_read32(g_cpu.A[7]) & 0xFFFFFFu;
            g_cpu.A[7] += 4;
        } else if (m68k_interp_step() != M68KI_OK) {
            fprintf(stderr, "WotES state continuation failed at $%06X\n", pc);
            abort();
        }
    }
}

/* Development-only new-character override. The release build option removes
 * these writes; the environment switch permits original-stat comparisons
 * in a testing build. No per-frame enforcement or save-load modification. */
static void test_character_stats(uint32_t address) {
#if WOTES_TEST_PARTY_STATS
    const char *mode = getenv("WOTES_TEST_PARTY_STATS");
    if (mode && !strcmp(mode, "0")) return;
    unsigned base = address & 0xFFFFu;
    if (base < 0x912Cu || base > 0x92EEu || (base-0x912Cu)%0x96u) return;
    for (unsigned i=0;i<6;++i) m68k_write8(0xFF0000u+base+82+i,18);
#else
    (void)address;
#endif
}

static int game_instruction_hook(uint32_t pc) {
    if (pc == 0xBA70u || pc == 0xBA80u) glue_save_checkpoint(pc);
    if (pc == 0x1FB60u) wotes_floor_items_select();
    if (pc == 0x21332u) wotes_floor_item_pickup(0);
    if (pc == 0x213B0u) wotes_floor_item_pickup(1);
    /* Target selection temporarily clears a readied slot and saves its ID
     * at $A8F8. The equipment menus reuse that byte for their refresh flag.
     * Preserve the ID on the guest stack for the duration of the target
     * picker, including nested menus. The backup belongs to guest state.
     * The entry and both exit sites
     * lie inside the original MOVEM frame; the extra frame is balanced
     * before $3F10 restores the slot and before the original epilogue.
     * The marker lets older mid-target save states finish without a pop. */
    if (pc == 0x3950u) {
        g_cpu.A[7] -= 8u;
        m68k_write32(g_cpu.A[7], 0x57544152u);
        m68k_write32(g_cpu.A[7]+4u, g_ram[0xA8F8]);
    }
    if ((pc == 0x3B68u || pc == 0x3D84u) &&
        m68k_read32(g_cpu.A[7]) == 0x57544152u) {
        m68k_write8(0xFFA8F8u, (uint8_t)m68k_read32(g_cpu.A[7]+4u));
        g_cpu.A[7] += 8u;
    }
    /* $2516 follows the default-party copy. $1800 follows the six rolled
     * abilities, before creation copies their caches and derives HP. */
    if (pc == 0x2516u)
        for (unsigned i=0;i<4;++i) test_character_stats(0xFF912Cu+i*0x96u);
    if (pc == 0x1800u) test_character_stats(g_cpu.A[4]);
    /* The inscription loop polls after its background updates and frame
     * wait. Repeated reads can consume A/B/C/Start's press edge before the
     * close checks at $D7AC. Retain rising edges across loop iterations,
     * then let the original BTST branches and window cleanup execute.
     * Directions and all other menus keep their original input handling. */
    if (pc == 0xD784u) s_inscription_buttons = g_ram[0x8009] & 0xF0u;
    if (pc == 0xD7ACu) {
        uint8_t held = g_ram[0x8009] & 0xF0u;
        g_cpu.D[0] |= held & (uint8_t)~s_inscription_buttons;
        s_inscription_buttons = held;
    }
    const char *show = getenv("WOTES_SHOW_INTRO");
    if (show ? strcmp(show,"1")==0 : wotes_menu_intro_enabled()) return 0;
    if (pc == 0x022468u) return 1;
    if (pc == 0x01675Cu) {
        g_cpu.SR |= SR_C;
        return 1;
    }
    return 0;
}

/* IRQ4 points to reset in the cartridge vector table. Dynamic RAM-vector
 * dispatch, if installed by the game, must be established during boot probes. */
static void entry(void) { recomp_call_addr(0x200u); }
static void vblank(void) { recomp_call_addr(0x9E6u); }
static void hblank(void) { recomp_call_addr(0x200u); }
static void frame_post(uint64_t frame) { wotes_automap_tick(frame); wotes_video_probe(frame); }

const GameSpec g_game_spec = {
    .video = &wotes_video,
    .video_default = "16:9",
    .display_name = "Warriors of the Eternal Sun - WotESRecomp v0.20.0",
    .short_name = "WotES",
    .expected_rom_crc32 = WOTES_ROM_CRC,
    .expected_rom_size = 0x200000u,
    .tier3_floor_default = 1,
    .call_entry_point = entry,
    .call_vblank = vblank,
    .call_hblank = hblank,
    .on_post_reset = cartridge_reset,
    .on_frame_post = frame_post,
    .sram_loaded = wotes_automap_load,
    .on_state_loaded = wotes_automap_reset_position,
    .state_build_id = "wotes-v0.20-states-v1",
    .state_size = state_size,
    .state_save = state_save,
    .state_load = state_load,
    .state_at_boundary = state_boundary,
    .resume_main_loop_pc = 0xBA70u,
    .call_resume = resume_state,
    .instruction_hook = game_instruction_hook,
};
