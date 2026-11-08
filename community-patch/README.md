# WotES Community Fixes v2.1

A community extension for **PlanetChili's Warriors of the Eternal Sun ROM Patch V2.0**.
This is an independent extension, not an official PlanetChili release.

## Apply the patch

1. Start with your **already Chili V2.0-patched USA/Europe ROM**.
2. Open `WotES Community Fixes v2.1 (for Chili V2.0).bps` in a BPS patcher,
   such as [Floating IPS](https://github.com/Alcaro/Flips).
3. Select that Chili V2.0 ROM as the input and save the result under a new name.
4. Open the new ROM in your Mega Drive emulator.

The extension applies **after** Chili V2.0, not directly to an unpatched ROM.
The required input is 2,097,152 bytes, CRC32 **405ED24B**, SHA-1
**2380b201b872f7bd2c66f2a4d82df86709332d55**. BPS checksum validation rejects
the wrong input. Do not apply the extension twice.

The output CRC32 is **34C1B865**, SHA-1
**0755b029387e17f588a00d1b74bfb4230efea7e8**. Its Genesis checksum is updated.
No ROM image is included in this package.

## Changes

- **STENCH indicator:** the original party HUD displays STENCH while its timed
  effect is active, when no higher-priority negative condition occupies the label.
  Poison and other existing labels retain their priority. A display-cache update
  tracks the effect appearing and disappearing without modifying condition flags.
- **Gauntlets of Ogre Power:** use their inventory artwork on dungeon floors,
  during the pickup animation, and as dropped items in the top-down view.
  The gauntlets get a separate sprite descriptor and four graphics tiles;
  other items that share the original yellow cube retain it.
- **Signed attack modifiers:** fix negative modifiers wrapping as unsigned
  numbers in the Chili-patched attack helper. Stench still applies its legitimate
  -2 penalty. It no longer causes the erroneous collapse to a 5% hit chance.

Stench duration, resistance checks, resting and priest treatment retain their
original behavior. The priest does **not** gain a new stench cure.
Starting-party stats and original presentation/startup are unchanged.

## Existing saves

Battery/in-game saves retain the same layout. Make a copy of your `.srm` file
and give the copy the same base filename as the newly patched ROM, if your
emulator uses filenames to associate saves. Start the patched ROM normally and
load through the original game's Load Game screen.

Old emulator save states may contain old graphics or execution state. Prefer
an in-game save and a fresh ROM start when testing a cartridge patch.

## What is not in this ROM patch

The Windows port's widescreen, automap, application menus, input remapping,
save states and audio backend are host features. Its SRAM mapping, Camp restore,
and native execution fixes are also specific to the port. Testing-only all-18
starting stats are not included. Other suspected game fixes need independent
ROM-level reproduction before inclusion.

## Verification and limits

See `VERIFY.md`. This is an experimental community patch. Local checks exercised it in the
project's interpreter and native runner. Physical Mega Drive compatibility
and a complete playthrough have not been verified.

## Credits and source

- Original game: Westwood Associates / SEGA and the original rights holders.
- Base patch: [PlanetChili, ROM Patch V2.0](https://github.com/planetchili/wotes_patch/releases/tag/V2.0).
- This package supplies the community extension and reproducible patch-building
  source. It does not grant rights to the original game or its assets.

`build_patch.py` reconstructs the extension from the exact input ROM. Example:

```text
python build_patch.py "your-chili-v2.md" --local-rom "WotES Community v2.1.md" --patch "rebuilt.bps"
```

Alternatively, the independent validator/applier can apply the supplied patch:

```text
python apply_bps.py "your-chili-v2.md" "WotES Community Fixes v2.1 (for Chili V2.0).bps" "WotES Community v2.1.md"
```

The applier validates all checksums and refuses to overwrite an existing output.
