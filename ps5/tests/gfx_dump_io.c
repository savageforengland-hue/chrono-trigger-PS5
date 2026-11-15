/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _GNU_SOURCE
#include "runtime/gfx_dump_io.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static int fail_write,fail_close,fail_rename;
size_t __real_fwrite(const void*,size_t,size_t,FILE*);
size_t __wrap_fwrite(const void*p,size_t a,size_t b,FILE*f){
    if(fail_write){errno=ENOSPC;return 0;}return __real_fwrite(p,a,b,f);
}
int __real_fclose(FILE*);
int __wrap_fclose(FILE*f){int r=__real_fclose(f);if(fail_close){fail_close=0;errno=EIO;return EOF;}return r;}
int __real_rename(const char*,const char*);
int __wrap_rename(const char*a,const char*b){if(fail_rename){errno=EACCES;return -1;}return __real_rename(a,b);}
int main(int argc,char**argv){
    assert(argc==2);char dir[1024],path[1100],temp[1200],error[640];
    snprintf(dir,sizeof dir,"%s/nested/a/b",argv[1]);assert(gfxdump_mkdirs(dir,error,sizeof error));assert(gfxdump_mkdirs(dir,error,sizeof error));
    SDL_Color pal[256];for(unsigned i=0;i<256;i++)pal[i]=(SDL_Color){(uint8_t)i,(uint8_t)(255-i),(uint8_t)(i^0x55),255};
    enum {W=513,H=257};uint8_t*px=malloc(W*H),*before=malloc(W*H);assert(px&&before);
    for(int y=0;y<H;y++)for(int x=0;x<W;x++)px[y*W+x]=(uint8_t)(x+3*y);
    memcpy(before,px,W*H);snprintf(path,sizeof path,"%s/bpp8.png",dir);
    assert(gfxdump_write_png(path,W,H,px,1,pal,error,sizeof error));assert(!memcmp(before,px,W*H));
    assert(gfxdump_verify_png(path,W,H,error,sizeof error));assert(!gfxdump_verify_png(path,W+1,H,error,sizeof error));
    snprintf(path,sizeof path,"%s/bpp4.png",dir);assert(gfxdump_write_png(path,33,17,px,0,pal,error,sizeof error));
    /* Failing replacements must leave the previous valid file and no temp. */
    snprintf(temp,sizeof temp,"%s.tmp",path);memset(px,77,W*H);
    fail_write=1;assert(!gfxdump_write_png(path,33,17,px,0,pal,error,sizeof error));assert(strstr(error,"No space"));fail_write=0;
    assert(access(temp,F_OK)!=0&&gfxdump_verify_png(path,33,17,error,sizeof error));
    fail_close=1;assert(!gfxdump_write_png(path,33,17,px,0,pal,error,sizeof error));assert(access(temp,F_OK)!=0);
    fail_rename=1;assert(!gfxdump_write_png(path,33,17,px,0,pal,error,sizeof error));fail_rename=0;
    assert(access(temp,F_OK)!=0&&gfxdump_verify_png(path,33,17,error,sizeof error));
    assert(!gfxdump_write_png(path,0,17,px,0,pal,error,sizeof error));
    snprintf(path,sizeof path,"%s/block",argv[1]);FILE*f=fopen(path,"w");assert(f);fputs("not a directory",f);fclose(f);
    snprintf(path,sizeof path,"%s/block/child",argv[1]);assert(!gfxdump_mkdirs(path,error,sizeof error));assert(strstr(error,"directory"));
    snprintf(path,sizeof path,"%s/broken.png",dir);f=fopen(path,"wb");assert(f);fputs("not PNG",f);fclose(f);assert(!gfxdump_verify_png(path,1,1,error,sizeof error));
    free(before);free(px);puts("PASS: indexed PNG output, block boundaries, immutable pixels, nested directories, corrupt/missing output and write/close/rename failure cleanup");return 0;
}
