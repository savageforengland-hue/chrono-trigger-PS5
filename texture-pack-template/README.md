# CTPS5 graphics pack template

Add your own replacement PNGs under `graphics/` using the naming/layout expected by the original Egg of Time graphics replacement loader. Read the original PC README for its graphics dump and replacement workflow. No extracted images are provided here.

Use 2x–8x replacements and keep original proportions. Change `id`, `name` and `version` in pack.json; the ID must be lowercase letters, digits, hyphens or underscores and cannot be `default`.

ZIP `pack.json`, this README, your license notice and `graphics/` at the archive root. Use `python3 ps5/install_texture_pack.py PACK.zip --data-root /path/to/local/eggoftime --activate`, then transfer the prepared data folder to the PS5. Include a license for your own art. Do not distribute extracted game textures.
