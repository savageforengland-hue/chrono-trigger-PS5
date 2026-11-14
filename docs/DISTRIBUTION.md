# Public source and local game builds

Power Pudu's original license distinguishes the MIT-licensed authored runtime, recompiler and tooling from ROM-derived output. It explicitly says: “Code generated from a ROM, extracted files, ROMs and saves are not part of this project and must not be distributed with it.” The original notice is preserved in `upstream/EGG-OF-TIME-LICENSE.md`.

The public CTPS5 repository, Pages download and source release contain authored project source, configuration, build tools, documentation, project artwork, licenses and a pristine copy of the original PC project source. They exclude ROMs, extracted game assets, saves, generated game C and the statically linked game executable.

The PS5 `eboot.bin` links the locally generated game code. It therefore contains ROM-derived code even when assets are loaded from a separate NDS file. It must not be described as a game-code-free runtime download. Keep locally generated artifacts out of the public repository and issue attachments.

`tools/check_public_source.py` checks tracked/public source paths and archive members for known forbidden artifacts. It is a packaging guard, not a general copyright classifier. Review all new assets and third-party notices before adding them.
