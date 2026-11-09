# Patch your ROM for v0.25.0

The Windows port requires **WotES Community Fixes v2.1**, an independent extension
to **PlanetChili's ROM Patch V2.0**. The two version numbers are separate:
v0.25.0 is the Windows port; v2.1 identifies the supported ROM extension.

## If you already have a Chili V2.0 ROM

1. Back up your ROM and in-game saves.
2. Download and extract `WotES-Community-Fixes-v2.1.zip` from the
   [v0.25.0 release](https://github.com/FourEyedGeek/WotESRecomp/releases/tag/v0.25.0).
3. Open a BPS patcher, such as [Floating IPS](https://github.com/Alcaro/Flips).
4. Choose **Apply Patch**, then select
   `WotES Community Fixes v2.1 (for Chili V2.0).bps`.
5. Select your **already Chili V2.0-patched USA/Europe ROM** as the input.
6. Save the patched ROM under a new name. Do not overwrite your backup.

The extension cannot be applied directly to an unpatched ROM, another hack,
or a ROM that already has Community v2.1. Keep checksum validation enabled.
If the patcher rejects the input, check the ROM version rather than forcing it.

## If you have an original USA/Europe ROM

First obtain [PlanetChili's official V2.0 patch](https://github.com/planetchili/wotes_patch/releases/tag/V2.0)
and follow its instructions. Then apply our BPS extension using the steps above.
Neither this repository nor its releases supplies a ROM.

## Verify the result

| Fingerprint | Chili V2.0 input | Community v2.1 output |
|---|---|---|
| Size | 2,097,152 bytes | 2,097,152 bytes |
| CRC32 | `405ED24B` | `34C1B865` |
| SHA-1 | `2380b201b872f7bd2c66f2a4d82df86709332d55` | `0755b029387e17f588a00d1b74bfb4230efea7e8` |

On Windows, `Get-FileHash -Algorithm SHA1 "your-patched-ROM.md"` checks SHA-1.
The included Python applier also validates input, output and patch checksums:

```text
python community-patch/apply_bps.py "your-chili-v2.md" "community-patch/WotES Community Fixes v2.1 (for Chili V2.0).bps" "WotES Community v2.1.md"
```

For the Windows source build, copy the resulting Community v2.1 ROM to
`local/wotes.md`, run `setup-deps.ps1`, then `build.ps1`.
For an emulator, open the new ROM normally. Prefer an in-game save after a
fresh start: older emulator save states may preserve old code or graphics.
If necessary, copy your `.srm` to match the new ROM's base filename.

## Included ROM fixes

- STENCH appears in the original party status label when no higher-priority
  negative condition occupies it.
- Gauntlets of Ogre Power use their inventory icon on dungeon floors, during
  pickup, and when dropped in the overhead view.
- Negative attack modifiers remain signed in the Chili attack helper.

Stench timing, resistance checks, rest and priest treatment remain unchanged.
This extension does not include widescreen, app menus, automapping, save states,
or testing-only all-18 starting stats. Those are separate port features.
See [patch details](community-patch/README.md) and [verification](community-patch/VERIFY.md).
