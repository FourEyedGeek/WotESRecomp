# Save states (v0.20.0)

Use the application Save State / Load State menus for four manual slots.
Replacing a manual slot asks for confirmation. Empty, damaged or incompatible
states are rejected without replacing the active game.

F5 writes the independent Quick Save slot; F6 restores it. The quick slot is
also listed in the menus. Quick Load is available from the startup menu.
Manual files are state_1.bin through state_4.bin; quick_state.bin is separate.
The previous F1-F8 numbered shortcuts have been removed.

Saving captures the next audited gameplay wait boundary. Loading restores the
CPU, RAM, video and sound state. Current in-game battery-save slots are preserved;
the persistent automap remains cumulative. Save states are independent of the
original game's Camp Save Game / Load Game system.

v0.20.0 uses the build identifier wotes-v0.20-states-v1. Earlier public/private
preview states are not accepted; keep them with their matching executable.
Normal in-game battery saves retain their existing format. Back up progress
before changing versions.
