# Current stats (v0.25.0)

Inventory and shop comparisons apply the combined modifiers from current
equipment and active timed effects. Positive and negative modifiers are included
together in hit, damage and AC. The calculator does not average possible
resistance outcomes or possible future combinations of conditions.

Hit is a nominal d20 rating against an ordinary AC0 opponent. Each successful
die face contributes five percentage points. It describes the current modifier
set, rather than predicting the cartridge random generator's actual distribution.
STENCH alone is identified as -2; multiple active effects are marked + EFFECTS.

All calculations run on private CPU/RAM copies. Checks for acquiring an already
stored timed condition are bypassed only in this display model. Remaining
unsupported conditional calculations show N/A or SPECIAL, not invented values.
Spells, abilities and empty action slots retain N/A. A and B are independent.

The weapon helper's context now uses the same zero-based record index as the
cartridge. This corrects previews for the +1 bow and +1 sling, including ranged
damage. It does not change combat damage or accuracy.

Validation covered no effects, Stench, Bless, Bless plus Stench, Blight plus
Stench, and Bless/Stench/Fear/Protection from Evil together, followed by stench
expiry. Both assigned weapons were checked. Inventory AC and armour-shop
current/proposed AC included the protection effect. Complete live RAM matched
the previous executable at all 21 comparison checkpoints.

This build uses the Community v2.1 ROM: CRC32 34C1B865,
SHA-1 0755b029387e17f588a00d1b74bfb4230efea7e8. The ROM and BPS patch are unchanged.
