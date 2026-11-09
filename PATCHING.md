# Patch your ROM: Community v2.3

Follow [community-patch/v2.3/README.md](community-patch/v2.3/README.md) for the complete guide and checksums.

1. Download and extract [WotES-Community-v2.3.zip](https://github.com/FourEyedGeek/WotESRecomp/releases/download/v2.3/WotES-Community-v2.3.zip).
2. Start with an **already PlanetChili V2.0-patched USA/Europe ROM**. If you only have the retail ROM, first apply [Chili's official V2.0 patch](https://github.com/planetchili/wotes_patch/releases/tag/V2.0).
3. Apply **WotES Community v2.3 - APPLY TO Chili V2.0.bps** with a BPS-compatible patcher such as Floating IPS.
4. Save the output to a new filename. Start normally and transfer progress through an in-game battery save.

Required input: 2,097,152 bytes, CRC32 `405ED24B`, SHA-1 `2380b201b872f7bd2c66f2a4d82df86709332d55`.

Result: 2,097,152 bytes, CRC32 `91EFC7F8`, SHA-1 `8757cf0e935656d98e1a29677e3e15ba069aa0e5`.

One cumulative patch includes v2.1 fixes, v2.2 action preservation and the v0.32 status screen. Do not apply it to an earlier Community/status ROM or patch twice. Wrong inputs are rejected; keep checksum validation enabled.

The ROM output matches the tested Complete Status v0.32 ROM. No game balance changes are added by the status screen. Old emulator states may contain obsolete pointers/stack layouts; keep backups and use in-game SRAM saves when moving versions.

## Older Windows source

The repository's Windows source remains v0.25.0 and requires Community v2.1, not this v2.3 ROM. Follow [PATCHING-v0.25.md](PATCHING-v0.25.md) if building that historical source release. The matching local Windows preview for v2.3 is v0.32.

No ROM, translated game source or executable is included in this repository or release.
