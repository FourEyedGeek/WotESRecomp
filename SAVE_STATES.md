# Save states

F1 through F4 save four slots; F5 through F8 load the corresponding slots.
The files `state_1.bin` through `state_4.bin` live in the application data
directory (beside the executable by default). Saving replaces that slot
atomically; a missing or incompatible load leaves gameplay unchanged.

States use a versioned, checksummed container. A request parks at the next
audited frame-wait entry ($BA70 or $BA80), before that entry changes registers
or the guest stack. The container stores CPU, work RAM, video, sound chips,
sound processor and adapter state. File states contain guest addresses, not
host stack pointers. On load, the adapter follows the saved guest stack and
uses the instruction floor for caller continuations between compiled entries.
Sound return-capture helpers retain their instruction-floor policy.

Cartridge battery memory is preserved across state loading, so Camp Save
slots are independent. Automap discoveries remain cumulative. The inscription
button latch is restored with the state. Save files are version-specific;
in-game battery saves keep their existing format.

Verification covered all four file slots in overhead and dungeon scenes,
restoring from a fresh process, party inventory/HP/ability data, dungeon pose
and subsequent turning. An isolated test also loaded a state while the current
battery image differed and verified that the current battery image survived.
