/* PS5 homebrew integration for Egg of Time. Local port, November 2026. */
#include <stdio.h>
#include <sys/stat.h>
#include <SDL3/SDL.h>
#include "config.h"
#include "diagnostics.h"
#include "memory.h"

/* Use the platform C library's thread flag rather than defining an exported copy. */
extern int __isthreaded;
extern int sceSystemServiceLoadExec(const char *, const char *const *);
extern int sceKernelUsleep(unsigned);

/* ApricotGS's native lifecycle: ordinary return/exit can be reported as a
 * crash. Ask the shell to close the title and wait for its asynchronous close. */
void catchReturnFromMain(int status) {
    char message[128];
    snprintf(message, sizeof message, "main status=%d; requesting shell closure", status);
    egg_ps5_diag_stage(110, message);
    int result = sceSystemServiceLoadExec("exit", NULL);
    snprintf(message, sizeof message, "shell-close request returned 0x%08x; waiting for shell", (unsigned)result);
    egg_ps5_diag_stage(111, message);
    for (;;) (void)sceKernelUsleep(100000);
}
static void ps5_log(void *data, int category, SDL_LogPriority priority, const char *message) {
    (void)data;
    (void)category;
    (void)priority;
    egg_ps5_diag_message(message);
}

bool egg_ps5_init(void) {
    __isthreaded = 1;
    if (!egg_ps5_memory_prepare()) return false;
    mkdir("/data/eggoftime", 0777);
    mkdir("/data/eggoftime/rom", 0777);
    mkdir("/data/eggoftime/mods", 0777);
    mkdir("/data/eggoftime/mods/packs", 0777);
    mkdir("/data/eggoftime/mods/graphics", 0777);
    SDL_SetLogOutputFunction(ps5_log, NULL);
    egg_ps5_config_load();
    int height = egg_ps5_config.output_height;
    char width_text[16], height_text[16];
    snprintf(width_text, sizeof width_text, "%d", height * 16 / 9);
    snprintf(height_text, sizeof height_text, "%d", height);
    SDL_SetHint("SDL_PS5_OUTPUT_WIDTH", width_text);
    SDL_SetHint("SDL_PS5_OUTPUT_HEIGHT", height_text);
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "ps5");
    SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "ps5");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_Log("CTPS5 HUD preview 4 (01.006.004): native PS5 build; Egg of Time 1.3.5 (USA/YQUE)");
    SDL_Log("output: %sx%s at 60 Hz; frame target: %d; internal scale: %d",
            width_text, height_text, egg_ps5_config.target_fps, egg_ps5_config.render_scale);
    return true;
}
