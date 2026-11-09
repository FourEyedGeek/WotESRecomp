# WotES Community v2.3 — Complete Status Effects

An independent community extension for **PlanetChili's Warriors of the Eternal Sun ROM Patch V2.0**. This is not an official Chili release. It contains the status-screen revision developed as v0.32, plus all Community v2.1/v2.2 fixes, in one BPS patch.

## Apply once to Chili V2.0

1. Back up your ROM and battery save.
2. Start with the **already Chili V2.0-patched USA/Europe ROM**.
3. In a BPS-compatible patcher such as [Floating IPS](https://github.com/Alcaro/Flips), select `WotES Community v2.3 - APPLY TO Chili V2.0.bps`.
4. Select the Chili V2.0 ROM as input. Save the result under a new filename.
5. Start the resulting ROM normally in your emulator or compatible Mega Drive setup.

Do not apply this to a clean retail ROM, Community v2.1/v2.2, or an earlier status-menu ROM. Use a fresh copy of Chili V2.0, not multiple community patches in sequence. Keep checksum validation enabled. If starting with the original retail ROM, first obtain [PlanetChili's V2.0 patch](https://github.com/planetchili/wotes_patch/releases/tag/V2.0) and follow its instructions.

| Fingerprint | Required Chili V2.0 input | Community v2.3 output |
|---|---|---|
| Size | 2,097,152 bytes | 2,097,152 bytes |
| CRC32 | `405ED24B` | `91EFC7F8` |
| SHA-1 | `2380b201b872f7bd2c66f2a4d82df86709332d55` | `8757cf0e935656d98e1a29677e3e15ba069aa0e5` |

The output is byte-for-byte identical to the tested Complete Status v0.32 ROM. v2.3 is the community cartridge release number; v0.32 identifies the matching Windows preview/status development revision.

## New status screen

Open **START → OPTIONS → STATUS EFFECTS**. The right-side portraits select the companion; the lower-left panel lists their active effects and remaining counters; the upper-left panel describes the highlighted effect.

- **C:** next companion. **Up/Down:** select effect. **START:** return to Options.
- Only active effects are listed, with six rows per page and scrolling for longer lists.
- 48 named mappings cover conditions, timed spells, relevant shared/area effects and continuous magic-item effects. Unknown sources have a generic fallback.
- Descriptions state verified numerical modifiers where applicable. Behavioral effects are described without invented percentages.
- The bottom control hints and timer legend are removed. The TIME column remains: counters are raw game ticks, not seconds; `--` means no timed source was found. Multiple sources of the same named effect share one row with the longest remaining counter.
- Light/Darkness flag labels are corrected, and a named source suppresses duplicate generic flag rows.

All 236 spell/action/item records are classified in [EFFECT-RECORDS.csv](EFFECT-RECORDS.csv). [EFFECT-AUDIT.md](EFFECT-AUDIT.md) lists mappings, descriptions, implementation and limits. Instant attacks, healing and permanent level changes are not fabricated as ongoing status effects.

## Cumulative fixes retained

- Preserve the readied action through targeting/equipment-menu transitions, preventing invalid weapon/ability restoration and resulting corruption.
- Display STENCH in the original HUD when no higher-priority negative label occupies it.
- Draw Ogre Power gauntlets using their inventory artwork on dungeon floors, during pickup flight and as overhead dropped items.
- Preserve signed negative attack modifiers in Chili's attack helper.

Original starting stats, intro, RNG, Stench duration/resistance, rest and priest treatment remain unchanged. The gauntlets retain Chili's +3 melee Strength contribution, equivalent to this cartridge's Strength18 contribution; no gauntlet rebalance is included. No missing-shop-spell or empty-spell-handler gameplay fix is claimed.

## Saves and Windows compatibility

The in-game battery-save layout is unchanged. Copy your existing `.srm` backup to match the new ROM filename if your emulator requires it, then start normally and use the original Load Game screen. Old emulator save states can contain obsolete code pointers/stack frames; transfer progress through in-game saves and create new states after upgrading.

This ROM patch does not add widescreen, automap, Windows menus or host save states. The Windows CAMP interpreter workaround remains a host feature. Older Windows releases expect their older ROMs; do not replace the v0.25 port's ROM with v2.3. The matching local Windows preview is v0.32 and is separate from this cartridge-only release.

## Rebuild and inspect

Python3 is sufficient; no compiler is needed to build the BPS. Supply your own exact Chili V2.0 ROM:

```text
python build_patch23.py "your-chili-v2.md" --output-rom "private-build/community-v2.3.md" --patch-dir "rebuilt-patches"
```

The builder validates the input, constructs v2.1 and v2.2 fixes, adds the v0.32 status screen, checks the expected target and independently decodes its BPS. Keep the output ROM private; distribute only the BPS and handwritten source/docs.

To apply without a graphical patcher:

```text
python apply_bps.py "your-chili-v2.md" "WotES Community v2.3 - APPLY TO Chili V2.0.bps" "community-v2.3.md"
```

The applier checks input, output and patch checksums and refuses to overwrite an existing output. Source files: `community21.py` (earlier fixes/assembler/BPS), `build_patch22.py` (action preservation), `build_status_audit032.py` (screen/source collection), `status_catalog032.py` (names/descriptions), and `build_patch23.py` (cumulative release).

## Testing, credits and licensing

See [VERIFY.md](VERIFY.md). Emulator and focused instruction tests pass; a complete playthrough, physical cartridge and FPGA testing have not been performed. The patch uses ordinary 68000 cartridge code and requires no Windows hooks.

Original game: Westwood Associates / SEGA and its rights holders. Base ROM patch: [PlanetChili V2.0](https://github.com/planetchili/wotes_patch). Community work is independent. Project-authored changes are offered under the included [PolyForm Noncommercial1.0.0 license](LICENSE.md), to the extent contributors hold applicable rights. See [LICENSING.md](LICENSING.md) for scope. No license here grants rights to original game content or relicenses Chili's work.

No ROM, game executable, extracted assets, generated game translation, player saves or diagnostic recordings are included.
