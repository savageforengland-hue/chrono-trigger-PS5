/* Optional high-resolution field artwork, composited with the existing DS OBJ layer.
 * Host thread only. Disabled unless mods/fields/active.ini explicitly enables a test scene. */
#pragma once
#include <SDL3/SDL.h>

void field_layers_init(SDL_Renderer *renderer);
int field_layers_ready(void);
/* world_x/y are the native-map coordinates of the viewport's top-left corner.
 * view_w/h are in original DS pixels, independent of image resolution. */
void field_layers_draw(SDL_Renderer *renderer, int foreground, const SDL_FRect *destination,
                       float world_x, float world_y, int view_w, int view_h);
void field_layers_quit(void);
