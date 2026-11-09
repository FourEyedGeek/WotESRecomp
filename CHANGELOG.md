# Changelog

## Community ROM v2.3 — 2026-11-10

## New in v2.3

- **START → OPTIONS → STATUS EFFECTS**: active effects for the selected companion, with descriptions and remaining counters.
- 48 named mappings cover conditions, timed spells, shared/area effects and continuous magic items. All 236 spell/action/item records are classified in the included audit.
- Verified numerical modifiers where applicable; behavioral effects described without invented percentages.
- C changes companion; Up/Down selects an effect; START exits. Longer lists scroll over six rows per page.
- Corrected Light/Darkness flag labels and suppressed duplicate generic condition rows.
- Removed the bottom control hints and timer legend. The TIME column remains; values are original game counters, not seconds, and -- means no timed source found.

## Earlier fixes included

- Preserve readied actions across targeting/equipment-menu transitions, preventing invalid item/ability restoration.
- STENCH HUD indicator without changing its duration, resistance, rest or priest treatment.
- Proper Ogre Power gauntlet artwork on dungeon floors, during pickup and as overhead dropped items.
- Correct signed negative attack modifiers in Chili's attack helper.

Starting stats, intro, RNG and Chili's gauntlet strength rules are unchanged. No spell-shop expansion or empty-handler gameplay repair is included.

## Saves and compatibility

Battery/in-game save layout is unchanged. Back up the SRAM save, match its filename to the new ROM if required, start normally and load through the game's Load Game screen. Transfer progress through in-game saves rather than old emulator states, which can retain obsolete code pointers and stack frames.

This is a **cartridge patch release**. Widescreen, automap, Windows menus and the Windows CAMP workaround are separate host features. The matching local Windows preview is v0.32; older published Windows source expects its older ROM.


Readable cumulative builder, source fingerprints and complete effect audit are in community-patch/v2.3. This cartridge release includes v2.1/v2.2 fixes and the v0.32 status revision; the older Windows source is unchanged.

## v0.25.0 — 2026-11-09

### Fixed

- Correct signed negative attack modifiers in the supported ROM extension.
- Fix stale or misleading shop hit ratings and ranged weapon record lookup,
  including +1 bow and +1 sling damage previews.
- Combine active buffs and penalties for AC, hit and damage in shops and Inventory,
  rather than averaging hypothetical condition/resistance outcomes.

### Added

- Inventory panel aligned with each character: AC and independent A/B HIT and DMG.
  Spells, abilities and unsupported calculations show N/A or SPECIAL.
- Community Fixes v2.1 BPS, reproducible builder, checksum-validating applier,
  verification notes and instructions for patching a Chili V2.0 USA/Europe ROM.
- Original-ROM STENCH label and gauntlet artwork on dungeon floors, in pickup
  animation and in the overhead dropped-item view.

### Compatibility and limits

- Requires Community v2.1 ROM: CRC32 34C1B865. Chili V2.0 alone is no longer the
  supported input. This is an independent extension, not an official Chili v2.1.
- File-state identifier is wotes-v0.25-states-v1. Transfer older progress using
  in-game battery saves, not older public/private file states.
- Original starting stats remain the public-build default.
- D20 HIT is a nominal rating vs AC 0, not a prediction of the original RNG.
  The ROM's RNG, stench resistance/timer and priest treatment are unchanged.
- Source and BPS only: no ROM, extracted assets, generated translation or game exe.

### Validation

- Current-effect fixtures cover no effect, Stench, Bless, combinations with
  Blight/Fear/Protection from Evil and expiry, both weapons and armour comparisons.
  All 21 checkpoints preserve live game RAM against the previous build.
- Independent BPS decode matches the target; wrong/corrupt inputs are rejected.
  ROM checks include status priority/cache behavior, 27 dungeon-floor cells,
  72 signed-modifier cases and native/interpreter artwork comparisons.
- Windows x64 remains the verified target. No full-game or physical-hardware
  compatibility guarantee is made.


## v0.20.0 — 2026-11-08

### Added

- Startup and pause menus, including Resume, Reset and Exit.
- Remembered keyboard/controller remapping, dead zone, intro toggle, aspect ratio,
  fullscreen, volume and mute settings.
- Controller connection/reconnection handling and mouse navigation in app menus.
- Save State and Load State menus with four manual slots, saved times,
  overwrite confirmation and empty/damaged-state rejection.
- Independent F5 Quick Save and F6 Quick Load, including loading from startup.
- Widescreen shop comparisons beside each character: armour/shield AC, normal
  weapon damage and AC 0 hit chance, class restrictions and comparison colours.

### Changed

- Replaced the old F1–F8 numbered state shortcuts with menu slots and F5/F6 quick actions.
- Kept original Camp controls separate from the Esc application pause menu.
- Preserved the original shop image at the left in 16:9 and its original 4:3 display.
- Retained original starting stats in public builds; all-18 testing stats stay opt-in.

### Fixed

- Missing Camp/Inventory cursors and skipped menu actions after Quick Load
  interrupted Rest Party or another suspended interpreter fallback.
- Clear the abandoned fallback guard when restoring the game execution stack.

### Validation and known limits

- Reproduced the Camp failure in the previous binary; the same sequence now
  restores cursors, Inventory and the original Save Game screen.
- Manual/quick save-state regressions and shop pixel/RAM comparison checks pass.
- Hit percentages use an ordinary AC 0 opponent; conditional enchantments
  remain enemy dependent. Full-game compatibility is still being tested.
- Source-only preview: no ROM, generated game translation or game executable.
- v0.20.0 file states use a new build identifier; retain older states with their older build.

# v0.10 â€” first Windows preview

- Widescreen exploration, supported dialogue and reward windows.
- First-person automap with current facing and a green starting tile.
- Restored music and sound effects.
- Four file-based save states, separate from in-game battery saves.
- Intro skip, dungeon transition, inscription and targeting fixes.
- Consistent gauntlets artwork on the floor and during pickup.
- Normal starting stats by default; source-only public packaging.

This release targets one specific patched ROM and remains an early preview.
Complete-game compatibility and other platforms are not yet verified.

No precompiled game executable or generated game translation is published.
