/* PS5 host settings; independent of the translated game code. */
#pragma once

typedef struct EggPs5Config {
    int output_height;
    int target_fps;
    int render_scale;
    char active_pack[64];
} EggPs5Config;

extern EggPs5Config egg_ps5_config;
void egg_ps5_config_load(void);
const char *egg_ps5_graphics_root(void);

int egg_ps5_graphics_root_count(void);
const char *egg_ps5_graphics_root_at(int index);
