/* SDL3 surface/file API stub for host-only full PNG unit tests. */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define SDLCALL
#define SDL_PIXELFORMAT_ABGR8888 0x16762004u
enum { SDL_PATHTYPE_DIRECTORY = 2 };
typedef struct SDL_PathInfo { int type; } SDL_PathInfo;
typedef struct SDL_Surface {
    int w, h, pitch;
    uint32_t format;
    void *pixels;
} SDL_Surface;
typedef enum SDL_EnumerationResult {
    SDL_ENUM_CONTINUE = 0, SDL_ENUM_SUCCESS = 1, SDL_ENUM_FAILURE = 2
} SDL_EnumerationResult;
typedef SDL_EnumerationResult (*SDL_EnumerateDirectoryCallback)(void *, const char *, const char *);
bool SDL_GetPathInfo(const char *path, SDL_PathInfo *info);
bool SDL_EnumerateDirectory(const char *path, SDL_EnumerateDirectoryCallback cb, void *userdata);
SDL_Surface *SDL_LoadPNG(const char *path);
void *SDL_LoadFile(const char *path, size_t *datasize);
SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, uint32_t format);
void SDL_DestroySurface(SDL_Surface *surface);
void *SDL_malloc(size_t count);
void *SDL_calloc(size_t count, size_t size);
void SDL_free(void *p);
const char *SDL_GetError(void);
