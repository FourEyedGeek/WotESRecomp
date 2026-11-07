# License scope and third-party rights

## Project-authored code

The project-authored handwritten integration, configuration, scripts and original
modifications in this repository are offered under the PolyForm Noncommercial
License 1.0.0 in LICENSE.md, to the extent the contributors hold applicable rights.
The standard license text is reproduced unchanged. Existing copyright notices
and any separately applicable terms are retained.

## GenesisRecomp and shared recompiler

The GenesisRecomp engine is fetched from mstan/segagenesisrecomp at the commit
pinned in setup-deps.ps1. Its existing project code retains LICENSE.md and its
upstream notices. The m68k-recomp-core submodule retains its own LICENSE text,
also reproduced in licenses/LICENSE-recompiler. The included patches modify
these components; they do not replace their existing licenses.

The upstream engine's additional grant is reproduced unchanged in
licenses/PERMISSIONS-upstream.md. It permits specified voluntary donations,
sponsorships and advertising for the upstream code it covers, subject to its
conditions. Its scope does not automatically extend to independently authored
project code, other dependencies or original game content.

## Other dependencies

Dependencies keep their individual licenses. The inventory and retained notices
are in licenses/THIRD-PARTY-LICENSES.md and the other files in licenses/. The
pinned dependency source trees fetched by setup-deps.ps1 retain their own full
license files. A project-wide license does not replace these licenses.

## Original game and cartridge patch

No license in this repository grants permission to use or redistribute the
original Dungeons & Dragons: Warriors of the Eternal Sun program, artwork,
music, text, characters, names or trademarks. Those rights remain with their
respective holders. The separately obtained Chili cartridge patch is not
relicensed or distributed by this project.

Generated C is a translation of game instructions, not automatically new
project-owned code. Recompilation and omission of the original ROM do not by
themselves establish rights to redistribute that translation or a compiled
game executable. The v0.20.0 publication therefore contains handwritten source,
configuration, build scripts and engine patches, without generated translations
or a precompiled game executable. Users supply their own supported ROM locally.
