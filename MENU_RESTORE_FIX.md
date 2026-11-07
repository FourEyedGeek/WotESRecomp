# Camp menu restore fix (0.20.0)

Loading a file state while an interpreter fallback is suspended discards its
native call stack. The fallback re-entry guard previously remained set because
the abandoned call never reached its normal cleanup. Later interior-label
fallbacks were skipped: the shared sprite builder lost the Camp/Inventory
cursor, and several Camp actions returned without opening their screen.

Reset the guard when restarting the game execution stack. The restored guest
CPU and stack continue normally and later fallback calls remain available.
No save format, game rules, ROM bytes or saved party data are changed.

A controlled sequence reproduced the missing cursor in the old binary:
load through the original in-game screen, enter Camp, choose Rest Party,
then load a file state before the rest routine returns. The sprite helper at
$659E was skipped in the old build. With the fix the same sequence restores
cursor sprites and opens Inventory, with no unhandled interior-label calls.
Manual/quick-state menu regression and shop comparison checks also pass.
