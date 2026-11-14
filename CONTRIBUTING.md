# Contributing

Start with docs/BUILDING.md, docs/CREDITS.md and LICENSING.md. Keep changes focused and explain the user-visible behavior and validation. Preserve original authorship and all component notices. Declare AI-assisted contributions when relevant, consistent with the original project's disclosure.

Run the ROM-free checks in `.github/workflows/checks.yml`. For PS5 display or input changes, include actual console observations and distinguish host tests from hardware validation. Do not claim a stable release, 4K60 performance or a full playthrough without evidence.

Do not commit ROMs, saves, extracted assets, generated game code, local game binaries or SDK downloads. Run `python3 tools/check_public_source.py` before submitting. Bug reports should include version, firmware / launcher versions, scene, steps and sanitized logs; never attach game dumps.
