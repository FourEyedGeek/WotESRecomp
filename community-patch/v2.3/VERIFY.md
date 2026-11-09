# Community v2.3 verification

- Rebuilt cumulatively from the exact Chili V2.0 ROM, independently reconstructed from the BPS1 data in the original Chili `.bst` file. Source SHA1:2380b201b872f7bd2c66f2a4d82df86709332d55.
- Rebuilt Community v2.1 and v2.2 as intermediate steps, then the v0.32 status revision. The final result exactly matches the already tested v0.32 ROM, CRC32:91EFC7F8, SHA1:8757cf0e935656d98e1a29677e3e15ba069aa0e5.
- Independently decoded the cumulative BPS and compared its output byte-for-byte. Wrong versions, truncated source, already-patched target and damaged patches are rejected. The Genesis header checksum is valid.
- All236 spell/action/item records classified; all51 nonzero-duration records and all11 non-inert continuous item records have mappings across48 named entries.
- Genesis Plus GX checked each of48 mappings, timers and unchanged party records. Combined-source fixtures cover aura range, distinct cave alignment for ground/party sources, shared effects, party-wide silence, item/flag deduplication, blindness/Light, multiple-page navigation and unknown-source fallback.
- Focused cartridge instruction execution checks numerical modifiers, haste/slow timing, light/dark blindness, Striking damage, missile protection, anti-magic gating and Chili's gauntlet helpers. The gauntlet +3 contribution and Strength18 table entry are unchanged by this release.
- Windows native checks for the same ROM cover descriptions, paging/wrap, held C, companion cycling, status save-state restoration and party-data preservation. Two fresh Rest/CAMP navigation cycles passed with the separate Windows workaround; this is not a ROM-level CAMP fix claim.
- Existing v2.2 action-preservation tests independently exercised the cartridge in Genesis Plus GX, confirming the preserved action and balanced stack through targeting exits.

Physical Mega Drive/FPGA compatibility and a complete-game playthrough are unverified. These are focused tests, not a claim that every original spell or game mechanic is implemented correctly. See EFFECT-AUDIT.md for handler and display limits.
