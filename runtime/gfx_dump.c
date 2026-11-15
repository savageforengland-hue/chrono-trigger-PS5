/* Graphics dump (System Settings > Dump graphics): the game's sprites, backgrounds and fonts as PNG files in
 * mods/graphics/dump, for graphics mods. It reads the ROM in memory (rt_rom.data: a translation's patched copy,
 * with the same-size graphics mods applied), on a thread of its own while the game runs. The formats: gfx_rom.c.
 *
 * Written (paths under dump/ are the ROM's, with .png added):
 *   sheets/<file>.png      each tile sheet (.NCGR, _ncg.bin), 16 tiles wide, in its first palette row
 *   frames/<file>/NNN.png  each cell of a .NCER, drawn from the sheet of the same name
 *   screens/<file>.png     each _nsc.bin, drawn from the _ncg.bin of the same name
 *   fonts/<file>.png       each message font (.fnt), its glyphs 16 to a row in cells of one size; pixel values 0-3
 *   fonts/<file>.txt       the font's advance widths and ASCII characters, by glyph
 *   palettes.txt           the palette each image was drawn with
 * The images are indexed: the pixels keep the game's palette indexes, with index 0 of each 16-colour row (of the
 * palette, for 8 bits per pixel) transparent. Without a palette for a sheet (character sprites: the game picks
 * their palettes in code) the image is a grey ramp, the indexes still exact. Graphics mods (gfx_mod.c) take the
 * same paths. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include <SDL3/SDL.h>

#include "gfx_dump.h"
#include "gfx_dump_io.h"
#include "gfx_rom.h"
#include "rt.h"

static SDL_AtomicInt running, finished, done_count, total_count, image_count;
/* Progress by the bytes of the files done (sprites with many frames take longest), for the time left. */
static SDL_AtomicInt done_bytes, total_bytes;
static char out_root[1024];
static char first_error[640];
static SDL_AtomicInt error_count;
static int abort_dump,repair_mode;
static FILE *dump_log;
static char last_directory[1024];
static FILE *palette_log;

static void failed(const char *message){
    if(!first_error[0])snprintf(first_error,sizeof first_error,"%s",message);
    SDL_AddAtomicInt(&error_count,1);abort_dump=1;
    rt_log("gfxdump: %s",message);
    if(dump_log){fprintf(dump_log,"ERROR: %s\n",message);fflush(dump_log);}
}
/* Make every parent through native filesystem calls; report the actual path. */
static int output_path(const char*kind,const char*name,const char*end,char*path,size_t capacity){
    char dir[1024],error[640];
    int n=snprintf(path,capacity,"%s%s/%s%s",out_root,kind,name,end);
    if(n<0||(size_t)n>=capacity){failed("Output path is too long");return 0;}
    snprintf(dir,sizeof dir,"%s",path);char*slash=strrchr(dir,'/');if(!slash){failed("Invalid output directory");return 0;}*slash=0;
    if(strcmp(dir,last_directory)){
        if(!gfxdump_mkdirs(dir,error,sizeof error)){failed(error);return 0;}
        snprintf(last_directory,sizeof last_directory,"%s",dir);
    }return 1;
}
static FILE *create(const char*kind,const char*name,const char*end,char*path,size_t capacity,const char*mode){
    if(!output_path(kind,name,end,path,capacity))return NULL;
    FILE*f=fopen(path,mode);if(!f){char message[1200];snprintf(message,sizeof message,"Cannot open %s: %s",path,strerror(errno));failed(message);}
    else if(chmod(path,0644)!=0){char message[1200];snprintf(message,sizeof message,"File permissions %s: %s",path,strerror(errno));fclose(f);f=NULL;failed(message);}
    return f;
}
static void save_png(const char*kind,const char*name,int w,int h,uint8_t*px,int bpp8,const GfxPalette*p,const char*from){
    char path[1024],error[640];if(abort_dump||!output_path(kind,name,".png",path,sizeof path))return;
    if(!gfxdump_write_png(path,w,h,px,bpp8,p->c,error,sizeof error)){failed(error);return;}
    SDL_AddAtomicInt(&image_count,1);
    if(palette_log&&fprintf(palette_log,"%s/%s.png\t%s\t%s\n",kind,name,from,p->from?p->from->path:"-")<0)failed("Cannot write palettes.txt");
}

static void save(const char *kind, const char *name, const GfxLayout *l, const GfxTiles *t, const GfxPalette *p,
                 const char *from) {
    uint8_t *px = calloc((size_t)l->w, (size_t)l->h);
    if(!px){failed("Image pixel allocation failed");return;}
    gfx_draw(l, t, px);
    save_png(kind, name, l->w, l->h, px, l->bpp8, p, from);
    free(px);
}

/* A message font: its glyphs as a sheet, and their widths and characters in a .txt beside it. */
static void dump_font(const GfxFile *f) {
    GfxBlob b = gfx_open(f);
    GfxFont font;
    if (gfx_read_font(&b, &font)) {
        int w, h;
        gfx_font_size(&font, &w, &h);
        uint8_t *px = calloc((size_t)w, (size_t)h);
        if(!px){gfx_close(&b);failed("Font pixel allocation failed");return;}
        gfx_font_draw(&font, px);
        GfxPalette p;
        gfx_font_palette(f->path, &p);
        save_png("fonts", f->path, w, h, px, 0, &p, f->path);
        free(px);
        if(abort_dump){gfx_close(&b);return;}
        char path[1024];
        FILE *txt = create("fonts", f->path, ".txt", path, sizeof path, "w");
        if (txt) {
            fprintf(txt, "# %s: glyph number (its place in the .png, %d to a row), advance width in pixels (- : no\n"
                         "# glyph), and the ASCII characters drawn with it\n", f->path, GFX_FONT_COLUMNS);
            for (int g = 0; g < font.count; g++) {
                const uint8_t *r = gfx_font_glyph(&font, g);
                if (r) fprintf(txt, "%d\t%d\t", g, r[0]);
                else fprintf(txt, "%d\t-\t", g);
                for (int c = 32; c < 127; c++) {
                    if ((int)gfx_le16(font.data + 12 + c * 2) == g) fprintf(txt, c == ' ' ? " space" : " %c", c);
                }
                fputc('\n', txt);
            }
            int okay=!ferror(txt);if(fclose(txt))okay=0;if(!okay)failed("Cannot finish font metadata");
        } else {
            rt_log("gfxdump: cannot write %s", path);
        }
    }
    gfx_close(&b);
}

/* The _nsc.bin `f` drawn from its _ncg.bin. */
static void dump_screen(const GfxFile *f) {
    char path[300];
    snprintf(path, sizeof path, "%.*s_ncg.bin", (int)(strlen(f->path) - 8), f->path);
    const GfxFile *gf = gfx_find(path);
    if (!gf) return;
    GfxBlob b = gfx_open(f), gb = gfx_open(gf);
    GfxTiles t;
    GfxLayout l;
    if (gfx_read_tiles(&gb, &t) && gfx_screen_layout(&b, t.bpp8, &l)) {
        GfxPalette p;
        gfx_palette_for(f->path, t.bpp8, &p);
        save("screens", f->path, &l, &t, &p, f->path);
        free(l.tiles);
    }
    gfx_close(&gb);
    gfx_close(&b);
}

/* A tile sheet, and the cells of the .NCER of the same name. */
static void dump_graphics(const GfxFile *f) {
    GfxBlob b = gfx_open(f);
    GfxTiles t;
    GfxLayout l;
    if (gfx_read_tiles(&b, &t) && gfx_sheet_layout(&t, &l)) {
        GfxPalette p;
        gfx_palette_for(f->path, t.bpp8, &p);
        save("sheets", f->path, &l, &t, &p, f->path);
        free(l.tiles);
        char path[300];
        snprintf(path, sizeof path, "%.*s.NCER", (int)(strlen(f->path) - 5), f->path);
        const GfxFile *cf = gfx_ends_with(f->path, ".NCGR") ? gfx_find(path) : NULL;
        if (cf) {
            GfxBlob cb = gfx_open(cf);
            for (uint32_t c = 0, n = gfx_cell_count(&cb); c < n&&!abort_dump; c++) {
                if (!gfx_cell_layout(&cb, c, &l)) continue;
                char name[300];
                snprintf(name, sizeof name, "%.*s/%03u", (int)(strlen(cf->path) - 5), cf->path, (unsigned)c);
                save("frames", name, &l, &t, &p, cf->path);
                free(l.tiles);
            }
            gfx_close(&cb);
        }
    }
    gfx_close(&b);
}

static int is_unit(const GfxFile *f) {
    return gfx_ends_with(f->path, ".NCGR") || gfx_ends_with(f->path, "_ncg.bin") || gfx_ends_with(f->path, "_nsc.bin") ||
           gfx_ends_with(f->path, ".fnt");
}

/* A unit's share of the work: its file, and its cells for a sheet. */
static int unit_bytes(const GfxFile *f) {
    uint32_t n = f->size;
    if (gfx_ends_with(f->path, ".NCGR")) {
        char path[300];
        snprintf(path, sizeof path, "%.*s.NCER", (int)(strlen(f->path) - 5), f->path);
        const GfxFile *cf = gfx_find(path);
        if (cf) n += cf->size * 32; /* frames cost more than the sheet */
    }
    return (int)SDL_min(n, 1u << 24);
}

/* Repair only the dump subtree. Never follow links outside exported assets. */
static void repair_tree(const char*path,int depth){
    char error[1200];
    if(depth>32){failed("Dump directory nesting exceeds the repair limit");return;}
    if(chmod(path,0755)!=0){snprintf(error,sizeof error,"Directory permissions %s: %s",path,strerror(errno));failed(error);return;}
    DIR*d=opendir(path);if(!d){snprintf(error,sizeof error,"Open directory %s: %s",path,strerror(errno));failed(error);return;}
    struct dirent*entry;
    for(;;){
        errno=0;entry=readdir(d);
        if(!entry){
            if(errno){snprintf(error,sizeof error,"Read directory %s: %s",path,strerror(errno));failed(error);}
            break;
        }
        if(!strcmp(entry->d_name,".")||!strcmp(entry->d_name,".."))continue;
        char child[1024];int n=snprintf(child,sizeof child,"%s%s%s",path,path[strlen(path)-1]=='/'?"":"/",entry->d_name);
        if(n<0||n>=(int)sizeof child){failed("Dump repair path is too long");continue;}
        struct stat st;if(lstat(child,&st)!=0){snprintf(error,sizeof error,"Inspect %s: %s",child,strerror(errno));failed(error);continue;}
        if(S_ISDIR(st.st_mode))repair_tree(child,depth+1);
        else if(S_ISREG(st.st_mode)){
            if(chmod(child,0644)!=0){snprintf(error,sizeof error,"File permissions %s: %s",child,strerror(errno));failed(error);continue;}
            if(gfx_ends_with(child,".png")){
                if(gfxdump_verify_png(child,0,0,error,sizeof error))SDL_AddAtomicInt(&image_count,1);
                else failed(error);
            }SDL_AddAtomicInt(&done_count,1);
        }
    }
    if(closedir(d)!=0){snprintf(error,sizeof error,"Close directory %s: %s",path,strerror(errno));failed(error);}
}

static int worker(void *arg) {
    (void)arg;
    Uint64 t0 = SDL_GetTicks();
    snprintf(out_root, sizeof out_root, "%smods/graphics/dump/", rt_data_dir());
    char log_path[1100],error[640];
    snprintf(log_path,sizeof log_path,"%sgraphics-dump.log",rt_data_dir());
    dump_log=fopen(log_path,"w");
    if(dump_log&&chmod(log_path,0644)!=0)rt_log("gfxdump: report permissions: %s",strerror(errno));
    rt_log("gfxdump: %s; output %s; report %s",repair_mode?"repairing existing dump permissions":"starting export",out_root,log_path);
    if(dump_log){fprintf(dump_log,"CTPS5 graphics dump\nOutput: %s\nAction: %s\nState: running\n",out_root,repair_mode?"repair existing permissions; no re-export":"export");fflush(dump_log);}
    if(repair_mode){repair_tree(out_root,0);goto finished_dump;}
    if(!gfxdump_mkdirs(out_root,error,sizeof error)){failed(error);goto finished_dump;}
    snprintf(log_path,sizeof log_path,"%spalettes.txt",out_root);
    palette_log=fopen(log_path,"w");
    if(!palette_log){failed("Cannot open dump/palettes.txt");goto finished_dump;}
    if(chmod(log_path,0644)!=0){failed("Cannot set palettes.txt permissions");goto finished_dump;}
    fprintf(palette_log,"# image\tROM file\tpalette (- : grey, none found)\n");
    int files = gfx_list_files(), total = 0, bytes = 0;
    for (int i = 0; i < files; i++) {
        if (is_unit(gfx_file(i))) total++, bytes += unit_bytes(gfx_file(i));
    }
    if(dump_log){fprintf(dump_log,"ROM files: %d\nGraphics units: %d\n",files,total);fflush(dump_log);}
    if(!files||!total){failed("No supported graphics files found in the loaded ROM");goto finished_dump;}
    SDL_SetAtomicInt(&total_bytes, bytes);
    SDL_SetAtomicInt(&total_count, total);
    for (int i = 0; i < files&&!abort_dump; i++) {
        const GfxFile *f = gfx_file(i);
        if (!is_unit(f)) continue;
        if (gfx_ends_with(f->path, "_nsc.bin")) dump_screen(f);
        else if (gfx_ends_with(f->path, ".fnt")) dump_font(f);
        else dump_graphics(f);
        SDL_AddAtomicInt(&done_bytes, unit_bytes(f));
        SDL_AddAtomicInt(&done_count, 1);
    }
finished_dump:
    if(palette_log){int okay=!ferror(palette_log);if(fclose(palette_log))okay=0;palette_log=NULL;if(!okay)failed("Cannot finish palettes.txt");}
    if(!SDL_GetAtomicInt(&image_count)&&!SDL_GetAtomicInt(&error_count))failed("No PNG files were produced");
    rt_log("gfxdump: %s: %d verified PNGs, %d errors, %.1f s, in %s",repair_mode?"access repair":"export", SDL_GetAtomicInt(&image_count), SDL_GetAtomicInt(&error_count),
           (double)(SDL_GetTicks() - t0) / 1000, out_root);
    if(dump_log){fprintf(dump_log,"Verified PNG files: %d\nErrors: %d\nState: %s\nElapsed seconds: %.1f\n",SDL_GetAtomicInt(&image_count),SDL_GetAtomicInt(&error_count),SDL_GetAtomicInt(&error_count)?"failed or partial":"complete",(double)(SDL_GetTicks()-t0)/1000);fclose(dump_log);dump_log=NULL;}
    SDL_SetAtomicInt(&finished, 1);
    SDL_SetAtomicInt(&running, 0);
    return 0;
}

static void start(int repair) {
    if (!SDL_CompareAndSwapAtomicInt(&running, 0, 1)) return;
    SDL_SetAtomicInt(&finished,0);SDL_SetAtomicInt(&error_count,0);first_error[0]=0;last_directory[0]=0;abort_dump=0;repair_mode=repair;
    SDL_SetAtomicInt(&done_count, 0);
    SDL_SetAtomicInt(&total_count, 0);
    SDL_SetAtomicInt(&done_bytes, 0);
    SDL_SetAtomicInt(&total_bytes, 0);
    SDL_SetAtomicInt(&image_count, 0);
    SDL_Thread *t = SDL_CreateThread(worker, "gfxdump", NULL);
    if (t) SDL_DetachThread(t);
    else {failed(SDL_GetError());SDL_SetAtomicInt(&finished,1);SDL_SetAtomicInt(&running,0);}
}

int gfxdump_running(void) { return SDL_GetAtomicInt(&running); }

void gfxdump_progress(int *done, int *total, float *share) {
    *done = SDL_GetAtomicInt(&done_count);
    *total = SDL_GetAtomicInt(&total_count);
    int bytes = SDL_GetAtomicInt(&total_bytes);
    *share = bytes ? (float)SDL_GetAtomicInt(&done_bytes) / (float)bytes : 0;
}

int gfxdump_finished(int *images) {
    if (!SDL_CompareAndSwapAtomicInt(&finished, 1, 0)) return 0;
    *images = SDL_GetAtomicInt(&image_count);
    return 1;
}

int gfxdump_errors(void){return SDL_GetAtomicInt(&error_count);}
const char*gfxdump_error(void){return first_error;}

void gfxdump_start(void){start(0);}
int gfxdump_repairing(void){return repair_mode;}
void gfxdump_repair_start(void){
    char path[1024];snprintf(path,sizeof path,"%smods/graphics/dump",rt_data_dir());
    struct stat st;if(lstat(path,&st)==0&&S_ISDIR(st.st_mode))start(1);
}
