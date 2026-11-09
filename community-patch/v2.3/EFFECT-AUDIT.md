# Complete Status v0.32: cartridge effect audit

This audit is specific to the Chili/Community USA/Europe cartridge used by this patch. Descriptions follow the cartridge instructions, not tabletop D&D assumptions. All 236 sixteen-byte spell/action/item records were classified. All 51 records with nonzero duration and all 11 non-inert type-6/type-7 continuous item records have display mappings. These are not 236 independent ongoing conditions: many records are instant damage, healing, attacks or ordinary equipment.

The screen has 48 named entries plus an unknown-source fallback. Only active effects are listed. Sources include the selected companion's five timed slots, shared companion spells, nearby ground auras, condition flags and continuously applied magic items. Duplicate generic flag rows are suppressed when a named source explains them. Shared sources use the original game's range and eligibility checks; this display update does not repair or reinterpret those rules.

The bottom control hints and timer legend have been removed. Up/Down, C and Start still work. TIME remains the raw game counter, not seconds. `--` means no timed source was available; it is not a promise that the effect can never end. If several sources map to one row, the longest remaining counter is shown. Unknown records retain a generic fallback rather than receiving invented bonuses.

## Coverage

- 144: instant action or ordinary equipment; not an ongoing companion status.
- 19: instant condition application (condition flags/secondary timed record).
- 51: timed or area effect.
- 6: inert record or action with no VM program.
- 5: instant permanent stat/level change; no ongoing status record.
- 11: continuous magic item.

## Display entries

Lower AC is better. Hit modifiers are attack-roll modifiers, not percentage points. Saving throw bonuses and fixed damage modifiers are stated separately. Effects without a fixed numerical AC/hit/damage modifier describe their behavior instead.

| Entry | Sources (zero-based record IDs) | Description |
|---|---|---|
| DEAD | Condition flags / HP | HP: 0.<br>CANNOT MOVE OR ACT. |
| DISEASE | 6, 189 | PREVENTS NORMAL HEALING.<br>NO EXTRA HIT PENALTY<br>IN THIS PATCHED ROM. |
| POISON | 36, 134, 196, 200, 204, 205, 206 | -1 HP PER POISON TICK.<br>CONTINUES UNTIL CURED. |
| PETRIFIED | 67, 197, 198 | TURNED TO STONE.<br>MOVEMENT/ACTIONS BLOCKED. |
| ASLEEP | 45 | ASLEEP.<br>MOVEMENT/ACTIONS BLOCKED. |
| IMMOBILISED | 46, 48, 151, 199, 201, 202, 207, 218, 228 | PARALYSIS/WEB/ENTANGLE.<br>MOVEMENT/ACTIONS BLOCKED. |
| BLIND | 203 | NORMAL ACTIONS IMPAIRED.<br>WITH LIGHT/DARK BLINDNESS:<br>-6 HIT; FOES GAIN +4 HIT. |
| FEAR | 9, 23, 127, 148, 235 | FORCES FLEEING BEHAVIOUR.<br>NO FLAT HIT/DAMAGE LOSS. |
| SILENCED | 16 | BLOCKS PARTY SPELLCASTING.<br>NO FLAT HIT/AC MODIFIER. |
| FEEBLEMIND | 62 | RESTRICTS CASTER ACTIONS.<br>NO FLAT HIT/AC MODIFIER. |
| CONFUSION | 58 | RANDOMISED COMBAT ACTIONS.<br>NO FIXED HIT % PENALTY. |
| LIGHT | 14, 17, 220, 223 | IF BLINDING: HIT -6.<br>ATTACKERS HIT BONUS: +4.<br>COUNTERS DARKNESS. |
| DARKNESS | 15, 18, 221, 224 | IF BLINDING: HIT -6.<br>ATTACKERS HIT BONUS: +4.<br>COUNTERS LIGHT. |
| HIDDEN | 10 | HIDDEN FROM MOST TARGETS.<br>ATTACKING REVEALS YOU.<br>NO FLAT HIT/AC MODIFIER. |
| STENCH | 214 | TROGLODYTE STENCH.<br>ATTACK MODIFIER: -2. |
| BLIGHT | 26 | ATTACK MODIFIER: -1.<br>DAMAGE MODIFIER: -1. |
| CURSED | 139 | ATTACK MODIFIER: -2.<br>DAMAGE MODIFIER: -2. |
| BLESS | 25 | ATTACK MODIFIER: +1.<br>DAMAGE MODIFIER: +1. |
| HEROISM | 133 | ATTACK MODIFIER: +4.<br>DAMAGE MODIFIER: +2. |
| GIANT STRENGTH | 136 | ATTACK MODIFIER: +3.<br>DAMAGE MODIFIER: +8. |
| INVULNERABILITY | 137 | AC MODIFIER: -2.<br>SAVING THROW BONUS: +2. |
| PROTECTION EVIL | 19, 20, 222, 226 | AC MODIFIER: -1.<br>SAVING THROW BONUS: +1. |
| SHIELD | 44 | AC BECOMES 4 IF BETTER.<br>LOWER AC IS BETTER. |
| HASTE | 52, 138 | ACTION SPEED: X2.<br>COUNTERS SLOW.<br>DOES NOT STACK. |
| SLOW | 53 | ACTION SPEED: HALF.<br>COUNTERS HASTE. |
| RESIST COLD | 24 | VS COLD SPELLS:<br>SAVING THROW BONUS: +2.<br>DAMAGE MODIFIER: -1. |
| RESIST FIRE | 28, 131, 142 | VS FIRE SPELLS:<br>SAVING THROW BONUS: +2.<br>DAMAGE MODIFIER: -1. |
| PROTECT MISSILES | 56 | BLOCKS BOW/SLING ATTACKS,<br>INCLUDING +1 VERSIONS.<br>NO MELEE AC BONUS. |
| ANTI MAGIC SHELL | 21, 143 | BLOCKS SPELL EFFECTS,<br>INCLUDING FRIENDLY SPELLS.<br>NO FIXED AC MODIFIER. |
| STRIKING | 29 | ATTACK DAMAGE BONUS: +1D6.<br>BYPASSES SOME NORMAL<br>WEAPON IMMUNITIES. |
| FLY | 51 | FLIGHT EFFECT ACTIVE.<br>NO FIXED AC/HIT/DAMAGE<br>BONUS IN THIS ROM. |
| WATER BREATHING | 57 | WATER EFFECT ACTIVE.<br>NO FIXED AC/HIT/DAMAGE<br>BONUS IN THIS ROM. |
| DETECT MAGIC | 11, 219, 152 | MAGIC DETECTION EFFECT.<br>NO AC/HIT/DAMAGE BONUS. |
| DETECTION | 27, 154, 155 | TRAP/DOOR DETECTION.<br>NO AC/HIT/DAMAGE BONUS. |
| HOLD PORTAL | 40 | PORTAL EFFECT ACTIVE.<br>NO WORKING PORTAL HANDLER<br>IN THIS ROM. |
| WALL OF FIRE | 60 | WALL SPELL ACTIVE.<br>NO WALL/DAMAGE HANDLER<br>IN THIS ROM. |
| OTHER CONDITION | Condition flags / HP | UNRECOGNISED CONDITION.<br>NO ASSUMED STAT MODIFIER. |
| FIRE RING | 119 (continuous item scan) | VS FIRE SPELLS:<br>SAVING THROW BONUS: +2.<br>DAMAGE MODIFIER: -1. |
| PROTECTION RING | 120 (continuous item scan) | AC -1; HIT +1; DAMAGE +1.<br>DUPLICATE RINGS DO NOT<br>ADD MORE OF THIS BONUS. |
| REGENERATION | 121 (continuous item scan) | +1 HP PER RECOVERY TICK.<br>CANNOT RESTORE THE DEAD.<br>DISEASE BLOCKS HEALING. |
| SPELL TURNING | 122 (continuous item scan) | SPELL-TURNING HANDLER<br>HAS NO EFFECT IN THIS ROM. |
| WATER WALKING | 123 (continuous item scan) | WATER EFFECT ACTIVE.<br>NO FIXED AC/HIT/DAMAGE<br>BONUS IN THIS ROM. |
| WEAKNESS | 124 (continuous item scan) | PER ITEM:<br>ATTACK MODIFIER: -1.<br>DAMAGE MODIFIER: -1. |
| SPEED BOOTS | 125 (continuous item scan) | ACTION SPEED: X2.<br>COUNTERS SLOW.<br>DOES NOT STACK. |
| DISPLACEMENT | 126 (continuous item scan) | AC MODIFIER: -2.<br>SAVING THROW BONUS: +2. |
| OGRE POWER | 128 (continuous item scan) | MELEE STRENGTH BONUS: +3.<br>REPLACES NORMAL STR BONUS<br>FOR HIT AND DAMAGE. |
| GIANT GIRDLE | 129 (continuous item scan) | ATTACK MODIFIER: +3.<br>DAMAGE MODIFIER: +8. |
| CREATURE REGEN | 209 (continuous item scan) | +1D3 HP PER RECOVERY TICK.<br>CANNOT RESTORE THE DEAD.<br>DISEASE BLOCKS HEALING. |

## Corrections and limits

- Light and Darkness flag labels were reversed in the preceding menu metadata. They now match the cartridge's flag-setting instructions. Blindness does not create separate Light and Darkness rows.
- Stench has a -2 attack modifier. No probabilistic average is used in this menu.
- Resist Fire/Cold apply +2 to the matching saving calculation and -1 in the matching spell damage calculation. They are not flat percentage reductions.
- Shield improves AC to 4 only when that is better. Haste uses the doubled action-timing branch; multiple haste sources do not multiply that branch.
- Ogre Power follows Chili's melee strength helpers: a +3 strength contribution replaces the normal strength contribution. It is not an extra +3 added to an existing strength bonus.
- Protection from Missiles blocks the bow/sling record IDs, including their +1 variants. Anti Magic Shell's spell dispatch check also rejects friendly spells.
- Hold Portal, Wall of Fire and the Spell Turning ring have empty relevant handlers in this cartridge. The descriptions say so rather than promise a benefit. Other detection/world behavior is not assigned invented combat bonuses.
- Instant level/stat drain has no persistent timer or condition flag from which a truthful active 'level drained' row could be reconstructed. It is classified in the record audit, not fabricated as an active status.
- Ordinary weapon and armour bonuses remain equipment statistics, not additional status rows. Summons and other world objects are not automatically companion conditions.
- This is a display audit, not a claim that every spell or original mechanic is fully implemented. Unknown mod records can use the fallback, but arbitrary new mod behavior cannot be inferred automatically.

## Implementation map

The readable builder contains the patched OPTIONS dispatch, the screen/input loop, source collection, metadata lookup and text drawing. The catalog holds names, flags, record IDs and descriptions. The screen uses a 1 KB stack workspace; it does not allocate persistent status data in the save layout.

- Record table: `$03E58C`, 236 records of 16 bytes.
- Selected companion: `$FF912C + 150 * (index - 1)`; condition flags at +104, five timed records at +92.
- Ground/area records: `$FF8CE4`; positions: `$FF8CFC`.
- New screen helpers: `$113000` through `$113FFF`; metadata `$114000`; flag map `$114800`; source map `$114C00`; text `$115000`.
- Outside the status-menu allocation, the v0.31-to-v0.32 difference is only the cartridge header checksum. Gameplay effect instructions are unchanged.

## Verification

A separate Genesis Plus GX emulator checked each of the 48 mappings, timers, portrait selection and unchanged party data. Combined-source tests cover aura range, party silence, item/flag deduplication, blindness, Light, multiple pages and unknown IDs. A focused instruction-level harness executes the actual cartridge calculations for the numerical modifiers, action timing, blindness, weapon immunity, magic blocking and Chili's Ogre Power helpers. The matching Windows build also checks navigation, held C, paging, save-state restoration and Rest/CAMP navigation. These checks do not guarantee the absence of every original-game bug.
