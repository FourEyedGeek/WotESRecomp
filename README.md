# WotESRecomp — v0.10 preview

A Windows recompilation project for Dungeons & Dragons: Warriors of the
Eternal Sun, built with [GenesisRecomp](https://github.com/mstan/segagenesisrecomp).
This is an unofficial early preview, not a complete-game compatibility guarantee.

## Source-only preview

v0.10 publishes the handwritten integration, configuration and engine patches.
There is no precompiled game download in this release. Build locally using your
own supported ROM, following the instructions below. ROMs, extracted game assets,
generated game translations, saves and diagnostic recordings are not included.

The current target requires the 2,097,152-byte USA/Europe Chili-patched ROM:
CRC32 `405ed24b`, SHA-1 `2380b201b872f7bd2c66f2a4d82df86709332d55`.
An unpatched ROM or a different patch revision is not supported by this build.
The cartridge patch must be obtained and applied separately.

## Features

- Widescreen top-down exploration with the party panel at the right.
- Wide overhead dialogue and combat reward messages.
- Original first-person scene plus a persistent exploration automap on the left.
- Music and sound effects, intro skipping, and four independent save states.
- Fixes for dungeon transitions, inscription dismissal, targeting action icons,
  and the gauntlets floor/pickup artwork.
- Normal starting ability scores in the release build.

## Controls

| Action | Keyboard |
|---|---|
| Move / navigate | Arrow keys |
| Genesis A / B / C | Z / X / C |
| Start / camp | Enter |
| Save state slots 1–4 | F1 / F2 / F3 / F4 |
| Load state slots 1–4 | F5 / F6 / F7 / F8 |
| Fullscreen | F11 or Alt+Enter |
| Turbo, without audio | Hold Tab |

## Progress

In-game saves use a `.srm` file named after the ROM beside the executable by
default. Automap discoveries use its `.automap` sidecar. File states use
`state_1.bin` through `state_4.bin`. Loading a state preserves the current
in-game save slots; automap exploration remains cumulative.
Saving again replaces that state slot. Back up progress before changing versions.
State files are version-specific. Earlier experimental states are not accepted
by v0.10; normal in-game battery saves retain their existing format.

## Limitations

Windows x64 is the current release target. A complete beginning-to-ending
playthrough has not been verified. First-person rendering retains its original
view; the wider layout adds the automap rather than revealing extra wall geometry.
Some menus retain the original layout and borders. Full input-remapping UI,
mobile versions, mod tools and a map editor are future work. Sound accuracy
still needs broader comparison with a reference implementation.

See [ROADMAP.md](ROADMAP.md) for proposed work. Report a bug with the version,
scene and reproduction steps. Do not upload a ROM or private diagnostic files.

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
The supported cartridge changes come from the Chili patch. The original game
is by Westwood Associates; its game content is not licensed by this project.
Project-authored integration code and modifications are offered under
[PolyForm Noncommercial 1.0.0](LICENSE.md). Existing upstream code retains its
original terms. See [LICENSING.md](LICENSING.md) for the component-by-component
scope, upstream additional permissions and dependency notices. These licenses
grant no rights to the original game's code, assets, music, text or trademarks.
Using a recompilation tool does not itself establish permission to redistribute
the translated game or a compiled game executable.
