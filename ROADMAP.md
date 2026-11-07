# Warriors of the Eternal Sun port roadmap

Proposed scope for WotESRecomp. This is a planning checklist, not a promise
that every feature will be implemented. Checked items describe the current
Windows development build; they do not imply a complete-game certification.

## 1. Preserve the complete game

- [x] Run the supported patched cartridge as a Windows executable.
- [x] Load graphics and game data from a user-supplied ROM.
- [ ] Complete a beginning-to-ending playthrough, including the ending.
- [ ] Verify every dungeon entrance, exit, staircase, scripted event and boss.
- [ ] Verify dialogue, shops, treasure, quests and character progression.
- [ ] Verify combat initiation, turns, targeting, retreat, victory and defeat.
- [ ] Verify attacks, spells, skills, status effects, resistances and damage.
- [ ] Verify equipment bonuses, item descriptions and ready-action icons.
- [ ] Distinguish original game bugs, patch changes and port regressions.
- [ ] Compare original behavior against a reference emulator where needed.

## 2. First public Windows preview: v0.10

- [ ] Publish a clean repository with credits, licenses and build instructions.
- [ ] Publish a portable Windows download without a ROM or personal saves.
- [ ] Give unsupported or incorrectly patched ROMs a clear error message.
- [ ] Document exactly which ROM version and patch the release supports.
- [ ] Restore normal starting stats by default in the public release.
- [ ] Keep a separate development build for easier testing.
- [ ] Clearly identify v0.10 as an early preview with known limitations.
- [ ] Include a short controls guide and instructions for transferring saves.
- [ ] Verify a clean first launch, save, quit, relaunch and load.
- [ ] Provide a straightforward way to report a reproducible bug.

## 3. Picture and screen layout

- [x] Extend top-down exploration for the current widescreen layout.
- [x] Keep the party status panel at the right edge.
- [x] Keep supported overhead dialogue and combat reward windows wide.
- [x] Preserve the original first-person scene with an automap beside it.
- [x] Keep unsupported menus/scenes in their original layout with borders.
- [ ] Add an in-app choice of original view and widescreen view.
- [ ] Add window-size, resolution and fullscreen controls.
- [ ] Preserve correct pixel proportions at every supported aspect ratio.
- [ ] Offer sharp integer scaling and optional smoother scaling.
- [ ] Support resizing and switching monitors without visual corruption.
- [ ] Check every menu, dialogue window and transition for layout problems.
- [ ] Investigate a genuinely wider first-person view; retain the current
      view as a fallback if the original scene data cannot support it cleanly.
- [ ] Investigate additional aspect ratios after 4:3 and 16:9 are reliable.

## 4. Sound and timing

- [x] Restore music and sound effects.
- [ ] Compare music tempo, pitch, instruments and effects against a reference.
- [ ] Check scene changes, battles and state loads for missing or stuck notes.
- [ ] Add master volume and mute controls.
- [ ] Investigate separate music/effect volume if the sound driver allows it.
- [ ] Keep gameplay speed consistent across refresh rates and slower devices.
- [ ] Pause or handle focus loss cleanly, with a configurable background policy.
- [ ] Provide optional audio latency/buffer settings only if useful in practice.

## 5. Saving, loading and protecting progress

- [x] Support normal in-game battery saves and loads.
- [x] Provide four file-based save states: F1–F4 save, F5–F8 load.
- [x] Keep state operations separate from current in-game save slots.
- [x] Restore states after restarting the application.
- [x] Show a brief confirmation for state operations.
- [ ] Test states during combat, targeting, dialogue, events and transitions.
- [ ] Add a save-state browser with slot names, dates and thumbnails.
- [ ] Make accidental replacement/loading harder, with optional confirmation
      or an undo backup rather than mandatory prompts on every action.
- [ ] Add automatic backups of normal saves and replaced state slots.
- [ ] Consider a separate autosave slot at safe points.
- [ ] Provide clear messages for missing, damaged and incompatible states.
- [ ] Define a state compatibility policy across versions and mod sets.
- [ ] Support configurable save locations and an easy Open Save Folder action.
- [ ] Add save export/import suitable for moving between supported platforms.
- [ ] Consider a shared portable progress format before promising cross-platform
      save-state compatibility; raw machine states are currently build-specific.

## 6. Controls and accessibility

- [x] Support the current keyboard controls and keyboard state shortcuts.
- [ ] Verify common Xbox, PlayStation and generic controllers end to end.
- [ ] Add an in-app remapping screen for keyboard and controller controls.
- [ ] Support multiple bindings, reset-to-default and conflict warnings.
- [ ] Remap save states, fullscreen, pause and turbo as well as game buttons.
- [ ] Add controller dead-zone settings and hot-plug/reconnect handling.
- [ ] Show prompts matching the active controller where practical.
- [ ] Investigate direct mouse selection in menus, inventories and maps.
- [ ] Design touch controls for mobile instead of relying on a keyboard layout.
- [ ] Support comfortable UI sizes and readable optional modern text overlays.
- [ ] Avoid relying only on color for automap start/current/important markers.
- [ ] Offer reduced flashing and adjustable repeat/hold behavior where feasible.

## 7. Automap

- [x] Build a dungeon survey as the party explores.
- [x] Show the automap on the left, with a green starting tile.
- [x] Show the current position and facing direction.
- [x] Persist discovered areas between sessions.
- [ ] Verify separate dungeon and floor identification throughout the game.
- [ ] Add a larger map view, zoom, pan and optional follow-player mode.
- [ ] Add player-written notes, labels and custom markers.
- [ ] Record doors, stairs, exits and landmarks only when legitimately observed.
- [ ] Add show/hide and orientation options.
- [ ] Define whether discoveries are shared or separate between campaigns.
- [ ] Add map export/import and a deliberate reset action with a backup.
- [ ] Consider an overworld map after dungeon mapping is dependable.

## 8. Optional improvements to the game

- [x] Skip the introductory presentation in the current build.
- [ ] Expose intro skipping as an ordinary application option.
- [ ] Keep an original-behavior preset alongside optional improvements.
- [ ] Make bug fixes and gameplay changes individually documented and selectable
      when they can safely be separated from the supplied ROM patch.
- [ ] Investigate better item/equipment explanations and visible derived stats.
- [ ] Investigate combat logs that explain hit rolls, damage and defenses.
- [ ] Consider party-management and inventory conveniences.
- [ ] Keep difficulty adjustments and testing cheats explicitly optional.
- [ ] Preserve deliberate game balance unless a selected mod changes it.

## 9. Mods

- [ ] Define a versioned mod format and load order.
- [ ] Add enable/disable controls with dependency/conflict checks.
- [ ] Offer bug-fix and gameplay-tweak mods before larger content changes.
- [ ] Support replacement graphics and, where practical, audio assets.
- [ ] Expose editable item, enemy, spell and class data as it becomes understood.
- [ ] Investigate new classes, skills, encounters and quests.
- [ ] Associate saves with the active mod set and explain incompatibilities.
- [ ] Validate mod data and fail safely when content is malformed.
- [ ] Preserve credits and separate mod content from the base game.

## 10. Map editor

- [ ] Start with a read-only viewer of understood overworld and dungeon data.
- [ ] Add tile painting, collision, doors, stairs and object placement.
- [ ] Add encounters, treasure and event editing as their formats are understood.
- [ ] Add undo/redo, copy/paste, validation and safe backups.
- [ ] Provide a quick test-in-game workflow.
- [ ] Save edits as separate mod packages rather than overwriting the original ROM.
- [ ] Investigate larger maps and new areas after existing-map edits are stable.
- [ ] Document format and engine limits before exposing unsupported edits.

## 11. Other platforms

- [ ] Linux: build, package, controllers, sound and save-location testing.
- [ ] Android: touch UI, controller support, storage permissions, suspend/resume,
      performance, battery use and installable packaging.
- [ ] iOS: touch UI, controller support, file import, suspend/resume and a viable
      signing/distribution route.
- [ ] Keep platform-specific code separate from shared game behavior.
- [ ] Test supported CPU architectures rather than assuming Windows snapshots
      or executable formats transfer directly.
- [ ] Consider macOS if there is interest after the named target platforms.

## 12. Maintenance and release quality

- [ ] Establish repeatable builds from pinned dependencies.
- [ ] Maintain targeted checks for previous transition, input, icon, audio,
      widescreen and save-state regressions.
- [ ] Keep clean test scenarios that do not require publishing personal saves.
- [ ] Add build automation that accepts privately supplied ROM inputs.
- [ ] Keep ROMs, generated translations, personal saves and diagnostic logs out
      of the public source repository and release upload.
- [ ] Show the application version in the UI and bug reports.
- [ ] Offer diagnostic export with a preview of exactly what it contains.
- [ ] Keep release notes, known limitations, dependency credits and licenses current.

## Suggested order

1. Release packaging and progress protection for v0.10.
2. Complete-game playtesting and fixes.
3. Settings, controls, accessibility and automap usability.
4. Linux and Android foundations, then iOS distribution work.
5. Documented mod support and a read-only map viewer.
6. Map editing and larger gameplay/content extensions.

New features can overlap where independent, but preservation of saves and the
ability to finish the original game take priority over expanding the scope.
