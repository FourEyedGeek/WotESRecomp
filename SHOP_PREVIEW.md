# Shop preview 0.20.0

In 16:9, recognized shop screens preserve the complete 320-pixel game image
at the left of the 398-pixel canvas. A 78-pixel comparison panel occupies the
right side, with one row for each party member. Other scene layouts retain
their existing presentation, and 4:3 retains the original shop screen.

The highlighted item is decoded from the original shop cursor coordinates
and stock table. Character names and equipment come from the live party.
Armour/shield comparisons replace that category on a private party copy;
the original game's armour and active-effect helpers calculate the result.
Weapon comparisons use the readied A weapon, falling back to B when A is
not a weapon. Hit chance enumerates all 20 natural rolls through the patched
cartridge hit helper against an ordinary AC 0 opponent. Damage uses the
patched strength calculation and minimum/maximum damage dice. Each trial
begins with the same prepared actor; trials cannot accumulate bonuses or
penalties. Conditional enchantments are explicitly left enemy dependent.

The calculator compiles a separately named instance of the existing floor
instruction semantics. It has its own CPU, RAM, and a small private patch
scratch buffer. It never calls the live bus, game hooks, scheduler, audio,
or random generator. Audited execution ranges, an instruction ceiling and
rejection of unexpected memory access make unsupported calculations fail
closed. Calculated rows are cached until the item or party changes.

Validation: restored shop state, navigated plate mail to shield, and checked
normal/+1 weapon comparisons in an isolated shop fixture. The native 320px
image and live RAM are identical to the existing menu-preview executable
at both captured navigation points. The 4:3 result is pixel-identical to
the original image. Armour restrictions and conditional-enchantment
fallbacks were exercised. Player files are not used as test output paths.
