#include "field_layers.h"
#include "rt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct FieldArt {
    SDL_Texture *background, *foreground;
    int bg_w, bg_h, fg_w, fg_h;
    int native_width, native_height, origin_x, origin_y;
    char scene[64];
} FieldArt;
static FieldArt art;

static char *strip(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) *--end = 0;
    return s;
}
static int valid_scene(const char *s) {
    size_t n = strlen(s);
    if (!n || n >= sizeof art.scene) return 0;
    for (size_t i = 0; i < n; i++)
        if (!isalnum((unsigned char)s[i]) && s[i] != '_' && s[i] != '-') return 0;
    return 1;
}
static SDL_Texture *load_image(SDL_Renderer *renderer, const char *path, int *w, int *h) {
    SDL_Surface *surface = SDL_LoadPNG(path);
    if (!surface) {
        rt_log("field_layers: cannot load %s: %s", path, SDL_GetError());
        return NULL;
    }
    *w = surface->w;
    *h = surface->h;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
    }
    return texture;
}

void field_layers_init(SDL_Renderer *renderer) {
    field_layers_quit();
    char config[1100], line[256];
    snprintf(config, sizeof config, "%smods/fields/active.ini", rt_data_dir());
    FILE *f = fopen(config, "r");
    if (!f) return;
    int enabled = 0;
    while (fgets(line, sizeof line, f)) {
        char *entry = strip(line);
        if (!*entry || *entry == '#' || *entry == ';') continue;
        char *equal = strchr(entry, '=');
        if (!equal) continue;
        *equal++ = 0;
        char *key = strip(entry), *value = strip(equal);
        if (!strcmp(key, "enabled")) enabled = !strcmp(value, "1");
        else if (!strcmp(key, "scene") && valid_scene(value))
            snprintf(art.scene, sizeof art.scene, "%s", value);
        else if (!strcmp(key, "native_width")) art.native_width = atoi(value);
        else if (!strcmp(key, "native_height")) art.native_height = atoi(value);
        else if (!strcmp(key, "origin_x")) art.origin_x = atoi(value);
        else if (!strcmp(key, "origin_y")) art.origin_y = atoi(value);
    }
    fclose(f);
    if (!enabled || !art.scene[0] || art.native_width <= 0 || art.native_height <= 0 ||
        art.native_width > 16384 || art.native_height > 16384) return;
    char path[1200];
    snprintf(path, sizeof path, "%smods/fields/%s/background.png", rt_data_dir(), art.scene);
    art.background = load_image(renderer, path, &art.bg_w, &art.bg_h);
    if (!art.background) return;
    snprintf(path, sizeof path, "%smods/fields/%s/foreground.png", rt_data_dir(), art.scene);
    FILE *fg = fopen(path, "rb");
    if (fg) {
        fclose(fg);
        art.foreground = load_image(renderer, path, &art.fg_w, &art.fg_h);
    }
    rt_log("field_layers: active test scene %s, native map %dx%d, artwork %dx%d",
           art.scene, art.native_width, art.native_height, art.bg_w, art.bg_h);
}

int field_layers_ready(void) { return art.background != NULL; }

/* Crop in native map coordinates before mapping into texture and target coordinates.
 * This leaves black beyond the edges, rather than stretching edge pixels. */
static void draw_image(SDL_Renderer *renderer, SDL_Texture *texture, int tw, int th,
                       const SDL_FRect *dest, float world_x, float world_y, int view_w, int view_h) {
    if (!texture || !view_w || !view_h) return;
    float left = world_x - art.origin_x, top = world_y - art.origin_y;
    float right = left + view_w, bottom = top + view_h;
    float x0 = SDL_max(left, 0.0f), y0 = SDL_max(top, 0.0f);
    float x1 = SDL_min(right, (float)art.native_width), y1 = SDL_min(bottom, (float)art.native_height);
    if (x1 <= x0 || y1 <= y0) return;
    SDL_FRect source = {x0 * tw / art.native_width, y0 * th / art.native_height,
                        (x1 - x0) * tw / art.native_width, (y1 - y0) * th / art.native_height};
    SDL_FRect target = {dest->x + (x0 - left) * dest->w / view_w,
                        dest->y + (y0 - top) * dest->h / view_h,
                        (x1 - x0) * dest->w / view_w, (y1 - y0) * dest->h / view_h};
    SDL_RenderTexture(renderer, texture, &source, &target);
}

void field_layers_draw(SDL_Renderer *renderer, int foreground, const SDL_FRect *destination,
                       float world_x, float world_y, int view_w, int view_h) {
    if (!field_layers_ready()) return;
    if (foreground)
        draw_image(renderer, art.foreground, art.fg_w, art.fg_h, destination, world_x, world_y, view_w, view_h);
    else
        draw_image(renderer, art.background, art.bg_w, art.bg_h, destination, world_x, world_y, view_w, view_h);
}
void field_layers_quit(void) {
    if (art.background) SDL_DestroyTexture(art.background);
    if (art.foreground) SDL_DestroyTexture(art.foreground);
    memset(&art, 0, sizeof art);
}
