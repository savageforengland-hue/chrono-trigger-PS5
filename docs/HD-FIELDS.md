# Experimental HD field image renderer

This is an **opt-in, first-pass renderer**, not yet an automatic replacement for every map. It loads a full-scene background and an optional foreground PNG at their native resolutions. Original game logic (collision, events, movement) still runs. Only the visible field scenery is replaced. On missing or invalid config, or outside the field, the original renderer remains active.

## PS5 installation (manual test)

Create \`/data/eggoftime/mods/fields/active.ini\`:

\`\`\`ini
enabled=1
scene=test
native_width=512
native_height=512
origin_x=0
origin_y=0
\`\`\`

Create \`/data/eggoftime/mods/fields/test/background.png\`.
Optionally create \`/data/eggoftime/mods/fields/test/foreground.png\` with transparent regions in front of the actors.

The \`native_width\` and \`native_height\` describe the represented area **in original game pixels**, not the artwork's output resolution. For example, \`background.png\` may be 4096x4096 while the represented map remains 512x512. \`origin_x\`/\`origin_y\` are the map coordinates covered by the artwork's top left.

The test mode is **manual** and applies to any normal field scene that the game visits, not one selected by map ID. Turn \`enabled=0\` when leaving the test map. Never enable this as a whole-game mod until scene selection is implemented.

The initial compositor extracts the DS OBJ pixels over the scene. Characters can therefore show above the new background and beneath the new foreground. **Known limitations:** DS object pixels are not yet re-rendered from HD sprite source textures; map-specific automatic detection, precise text-box/lighting compositing, and image paging are not yet implemented. Camera mapping must be calibrated for each test location. Moving backgrounds or complex battle effects are not supported in this initial pass.

Test on a copy of your save and keep a known-good \`eboot.bin\`. The source is not a verified PS5 build until compiled with the project's PS5 SDK and tested on actual hardware. 
