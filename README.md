# WotESRecomp - v0.20.0 preview

A Windows recompilation project for Dungeons & Dragons: Warriors of the
Eternal Sun, built with [GenesisRecomp](https://github.com/mstan/segagenesisrecomp).
This is an unofficial early preview, not a complete-game compatibility guarantee.

## Source-only preview

v0.20.0 publishes the handwritten integration, configuration and engine patches.
There is no precompiled game download in this release. Build locally using your
own supported ROM, following the instructions below. ROMs, extracted game assets,
generated game translations, saves and diagnostic recordings are not included.

This port targets **(ROM Patch) Dungeons & Dragons: Warriors of the Eternal Sun,
V2.0**, by **PlanetChili** ([planetchili/wotes_patch](https://github.com/planetchili/wotes_patch)).
Get the patch from the [official V2.0 release](https://github.com/planetchili/wotes_patch/releases/tag/V2.0)
and apply it separately to your own USA/Europe ROM.

The resulting supported ROM is 2,097,152 bytes:
CRC32 `405ed24b`, SHA-1 `2380b201b872f7bd2c66f2a4d82df86709332d55`.
An unpatched ROM or a different patch revision is not supported by this build.
The cartridge patch must be obtained and applied separately.

## Features

- Startup menu: Start Game, Load State, Settings and Exit.
- Pause menu: Resume, Save State, Load State, Reset Game, Settings and Exit.
- Keyboard/controller remapping, controller hotplug support and dead-zone settings.
- Remembered settings for 4:3/16:9, fullscreen, volume/mute and the game intro.
- Four manual save-state slots plus an independent Quick Save slot.
- Widescreen overhead exploration, dialogue and combat reward messages.
- Persistent dungeon automap on the left, with a green starting tile.
- Widescreen shops with per-character armour/shield and normal weapon comparisons.
- Music and sound effects; the earlier transition, inscription, targeting and item-art fixes.
- Fixed missing Camp/Inventory cursors and skipped Camp actions after loading during Rest Party.
- Original starting ability scores by default; development-only all-18 stats are disabled.

## Controls

| Action | Default keyboard input |
|---|---|
| Move / original game menus | Arrow keys |
| Genesis A / B / C | Z / X / C |
| Start / original Camp menu | Enter |
| Application pause menu / back | Esc |
| Quick Save / Quick Load | F5 / F6 |
| Manual save/load slots | Application Save State / Load State menus |
| Fullscreen | F11 or Alt+Enter |
| Silent turbo | Hold Tab |

Application menus also support the mouse and a controller. Controller Back/View
opens pause; the south face button selects and the east face button goes back.
Application menu navigation stays fixed when game controls are remapped.
F1–F8 no longer act as the old numbered save/load shortcuts.

## Shops

In 16:9, the original shop image sits on the left. The added right panel compares
each party member with the highlighted item. Armour/shields show current and
prospective AC; lower is better. Weapons compare the readied A weapon, or B when
A is not a weapon, with the highlighted weapon's damage and hit chance against
an ordinary **AC 0** opponent. Real enemy defences and special effects can change
these results. Conditional enchantments are marked as enemy dependent. Class
restrictions and improvement/downgrade colours are shown. Browsing comparisons
does not equip or purchase an item.

## Progress and compatibility

In-game battery saves remain separate from file save states. The battery save
uses a ROM-named `.srm`; the automap uses its `.automap` sidecar. Manual states
use `state_1.bin` through `state_4.bin`; Quick Save uses `quick_state.bin`.
Loading a state preserves current battery slots and cumulative automap progress.
The menus provide overwrite confirmation and reject empty/damaged slots.
Settings are remembered in `settings.ini`. Intro changes apply on the next
Start or Reset.

Back up progress before changing versions. v0.20.0 has a new file-state build
identifier and does not accept v0.10 or private-preview state files. Normal
in-game battery saves retain their existing format. Existing characters keep
the stats stored in their saves.

## Validation and limitations

Automated local checks cover manual/quick state menus, rejected loads, independent
quick saves, shop comparisons, preservation of original shop pixels and game RAM,
4:3 presentation, and the reproduced Rest Party/Quick Load Camp-menu failure.
Input settings were also tested with a virtual controller; broader physical
controller and sound-accuracy testing remains useful.

Windows x64 is the verified target. A complete beginning-to-ending playthrough
has not been verified. First-person rendering keeps its original view and adds
an automap. Some game screens keep the original layout and borders. Conditional
shop enchantment comparisons, other platforms, mod management and a map editor
remain future work. See [CHANGELOG.md](CHANGELOG.md) and [ROADMAP.md](ROADMAP.md).
Bug reports should include the version and reproduction steps. Do not upload
ROMs or private saves/diagnostic recordings.

## Build from source

Install Git, Visual Studio 2022 or 2026 with Desktop development with C++, and
its CMake tools. Run `setup-deps.ps1` to fetch pinned upstream dependencies and
apply the included engine/core patches. Put your supported ROM at `local/wotes.md`,
then run `build.ps1`. The translation is generated locally from your ROM.
Generated C and ROM inputs are deliberately excluded from this repository.

The default build excludes the development-only all-18-stat override.
Existing characters keep the stats stored in their saves.

## Credits and licensing

GenesisRecomp and the shared recompiler: Matthew Stan and upstream contributors.
The supported cartridge changes come from **PlanetChili's Warriors of the Eternal Sun
ROM patch V2.0**. Credit and patch downloads: [planetchili/wotes_patch](https://github.com/planetchili/wotes_patch/releases/tag/V2.0). The original game
is by Westwood Associates; its game content is not licensed by this project.
Project-authored integration code and modifications are offered under
[PolyForm Noncommercial 1.0.0](LICENSE.md). Existing upstream code retains its
original terms. See [LICENSING.md](LICENSING.md) for the component-by-component
scope, upstream additional permissions and dependency notices. These licenses
grant no rights to the original game's code, assets, music, text or trademarks.
Using a recompilation tool does not itself establish permission to redistribute
the translated game or a compiled game executable.
