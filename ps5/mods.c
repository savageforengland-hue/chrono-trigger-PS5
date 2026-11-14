#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "config.h"

const char *egg_ps5_graphics_root(void) {
    static char root[256];
    if (strcmp(egg_ps5_config.active_pack, "default")) {
        snprintf(root, sizeof root, "/data/eggoftime/mods/packs/%s/graphics/", egg_ps5_config.active_pack);
        SDL_PathInfo info;
        if (SDL_GetPathInfo(root, &info) && info.type == SDL_PATHTYPE_DIRECTORY) {
            SDL_Log("graphics pack: %s", egg_ps5_config.active_pack);
            return root;
        }
        SDL_Log("graphics pack '%s' absent; using default graphics folder", egg_ps5_config.active_pack);
    }
    return "/data/eggoftime/mods/graphics/";
}
