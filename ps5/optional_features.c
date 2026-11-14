/* Core-only PS5 build. Original game cutscenes and sound remain in use.
 * Music archive identification is from Power Pudu's MIT-licensed music.c. */
#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "cheevos.h"
#include "dispatch.h"
#include "display.h"
#include "movie.h"
#include "rt.h"
#include "shader.h"
#include "sound.h"
#include "translation.h"

#define PLAYERS 16
#define MUSIC_GAIN 1.0f /* a replacement at full volume, against the game's mix */

/* ---- the sound archive's music sequences ---- */

typedef struct Bgm {
    char name[16];
    const uint8_t *data; /* the sequence's data (what the driver gets the address of), in the ROM image */
    uint32_t size;
    int volume;          /* the archive's volume for it, which the game sets as the player's volume */
} Bgm;

static Bgm bgms[256];
static int bgm_count, archive_read;
static int is_bgm[PLAYERS]; /* each player's sequence is a BGM (whether or not it is replaced) */

static uint32_t le32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static uint16_t le16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }

/* Finds the SDAT in the ROM image and lists its sequences named BGM_*. */
static void read_archive(void) {
    archive_read = 1;
    const uint8_t *rom = rt_rom.data;
    size_t n = rt_rom.size;
    for (size_t at = 0; at + 0x40 <= n; at += 4) {
        const uint8_t *s = rom + at;
        if (memcmp(s, "SDAT", 4) || le16(s + 4) != 0xFEFF || le32(s + 8) > n - at) continue;
        size_t size = le32(s + 8);
        uint32_t symb = le32(s + 0x10), info = le32(s + 0x18), fat = le32(s + 0x20);
        if (!symb || symb + 0x10 > size || info + 0x40 > size || fat + 0x10 > size) continue;
        uint32_t names = symb + le32(s + symb + 8), infos = info + le32(s + info + 8);
        uint32_t count = SDL_min(le32(s + infos), le32(s + names));
        uint32_t files = le32(s + fat + 8);
        for (uint32_t i = 0; i < count && bgm_count < (int)SDL_arraysize(bgms); i++) {
            uint32_t name = le32(s + names + 4 + 4 * i), entry = le32(s + infos + 4 + 4 * i);
            if (!name || !entry || symb + name >= size || info + entry + 8 > size) continue;
            const char *sym = (const char *)s + symb + name;
            if (strncmp(sym, "BGM_", 4)) continue;
            uint32_t file = le16(s + info + entry);
            if (file >= files) continue;
            uint32_t off = le32(s + fat + 12 + 16 * file), len = le32(s + fat + 16 + 16 * file);
            if (off + len > size || len < 0x1C || memcmp(s + off, "SSEQ", 4)) continue;
            uint32_t data = le32(s + off + 0x18);
            if (data >= len) continue;
            Bgm *b = &bgms[bgm_count++];
            snprintf(b->name, sizeof b->name, "%s", sym);
            b->data = s + off + data;
            b->size = len - data;
            b->volume = s[info + entry + 6];
        }
        rt_log("music: %d music sequences in the sound archive", bgm_count);
        return;
    }
    rt_log("music: no sound archive in the ROM");
}

/* The music sequence whose data is at `addr` in game memory, or NULL. */
static const Bgm *identify(uint32_t addr) {
    if (!archive_read) read_archive();
    for (int i = 0; i < bgm_count; i++) {
        const Bgm *b = &bgms[i];
        uint32_t k = 0;
        while (k < b->size && rt_r8(addr + k) == b->data[k]) k++;
        if (k == b->size) return b;
    }
    return NULL;
}


void music_prepare(int pno, uint32_t seq_data) {
    if (pno >= 0 && pno < PLAYERS) is_bgm[pno] = identify(seq_data) != NULL;
}
void music_stop(int pno) { (void)pno; }
int music_replaced(int pno) { (void)pno; return 0; }
int music_is_bgm(int pno) { return pno >= 0 && pno < PLAYERS && is_bgm[pno]; }
void music_player(int pno, int running, int db) { (void)pno; (void)running; (void)db; }
void music_mix(float *out, uint32_t frames, int speed) { (void)out; (void)frames; (void)speed; }
int movie_draw(SDL_Renderer *ren) { (void)ren; return 0; }
void rt_movie_play(CPU *cpu) { rt_call_wrapped(cpu, "ov017", 0x02198d20u); }
void rt_movie_play_scene(CPU *cpu) { rt_call_wrapped(cpu, "ov017", 0x02198da0u); }
void shader_scan(void) {}
int shader_init(SDL_Renderer *ren) { (void)ren; return 0; }
int shader_count(void) { return 0; }
const char *shader_key(int i) { (void)i; return ""; }
const char *shader_label(int i) { (void)i; return ""; }
int shader_run(int slot, int i, SDL_Texture *in, SDL_Texture *out, uint64_t frame) {
    (void)slot; (void)i; (void)in; (void)out; (void)frame; return 0;
}
int translation_apply(char *err, size_t size) { if (size) err[0] = 0; return 0; }
void cheevos_init(void) {}
void cheevos_frame(void) {}
void cheevos_reset(void) {}
void cheevos_poll(void) {}
int cheevos_state(void) { return CHEEVOS_LOGGED_OUT; }
const char *cheevos_user(void) { return ""; }
void cheevos_login(const char *user, const char *password) { (void)user; (void)password; }
void cheevos_logout(void) {}
int cheevos_list(const CheevosItem **items) { *items = NULL; return 0; }
const uint32_t *cheevos_badge(const char *url) { (void)url; return NULL; }
