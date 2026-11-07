# Changelog

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
