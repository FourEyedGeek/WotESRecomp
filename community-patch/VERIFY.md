# Verification

- Independently decoded the BPS output and compared it byte for byte with the
  built target ROM. Wrong inputs and damaged patches were rejected.
- Checked the updated Genesis checksum and both source/target fingerprints.
- Executed cartridge helper checks for stench in all 20 combinations of four
  party records and five effect slots. Tested label selection and priority,
  effect onset/removal cache changes, and unchanged actor condition flags.
- Exercised all 27 dungeon-floor item cells for gauntlet and unrelated-item paths.
- Verified the original eleven sprite descriptors and shared cube graphics are
  retained, with only item129 selecting the new gauntlet descriptor.
- Tested 72 native attack-modifier cases: rolls2 through19, with and without
  gauntlets, and with and without a -2 penalty. Results matched signed arithmetic.
- Checked the priest's original eligibility and healing routines retain stench.
- Rendered actual gameplay with the original HUD: STENCH present, POISON taking
  priority, STENCH reappearing after poison clears, and no label after expiry.
- Inspected visible top-down gauntlets beside an unrelated yellow-cube item.
- Verified gauntlet graphics in dungeon floor distance views and the pickup
  flight's active sprite. Pickup still transfers the item into inventory.
- Native and interpreter screenshots matched byte for byte at near, middle,
  far and completed-pickup checkpoints.

No physical hardware or independent-emulator compatibility claim is made.
