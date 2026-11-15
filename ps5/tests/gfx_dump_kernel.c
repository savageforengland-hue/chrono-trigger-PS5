/* SPDX-License-Identifier: GPL-3.0-or-later
 * Exercise the EGG_PS5 adapter with a separate kernel-path provider. Ordinary
 * pathname APIs deliberately fail, modelling a different/inaccessible route.
 * This is an API-routing test on Linux, not a PS5 filesystem emulation. */
#define _POSIX_C_SOURCE 200809L
#include "runtime/gfx_dump_io.h"
#include <SDL3/SDL.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
static int opens,mkdirs,syncs,renames,denied_open,fail_sync;
static int result(int r){return r<0?(int)(0x80020000u|(unsigned)errno):r;}
int __real_mkdir(const char*,mode_t);
int __real_chmod(const char*,mode_t);
int __real_rename(const char*,const char*);
int __real_unlink(const char*);
FILE*__wrap_fopen(const char*p,const char*m){(void)p;(void)m;errno=EACCES;return NULL;}
int __wrap_mkdir(const char*p,mode_t m){(void)p;(void)m;errno=EACCES;return -1;}
int __wrap_chmod(const char*p,mode_t m){(void)p;(void)m;errno=EACCES;return -1;}
int __wrap_rename(const char*a,const char*b){(void)a;(void)b;errno=EACCES;return -1;}
int __wrap_unlink(const char*p){(void)p;errno=EACCES;return -1;}
int sceKernelOpen(const char*p,int f,unsigned m){opens++;if(denied_open){errno=EACCES;return result(-1);}return result(open(p,f,(mode_t)m));}
int sceKernelClose(int fd){return result(close(fd));}
int sceKernelMkdir(const char*p,unsigned m){mkdirs++;return result(__real_mkdir(p,(mode_t)m));}
int sceKernelChmod(const char*p,unsigned m){return result(__real_chmod(p,(mode_t)m));}
int sceKernelRename(const char*a,const char*b){renames++;return result(__real_rename(a,b));}
int sceKernelUnlink(const char*p){return result(__real_unlink(p));}
int sceKernelFstat(int fd,struct stat*s){return result(fstat(fd,s));}
int sceKernelFsync(int fd){syncs++;if(fail_sync){errno=EIO;return result(-1);}return result(fsync(fd));}
int main(int argc,char**argv){
    assert(argc==2);assert(SDL_Init(0));umask(0077);
    char dir[1024],path[1100],error[640];snprintf(dir,sizeof dir,"%s/kernel/frames",argv[1]);
    assert(gfxdump_mkdirs(dir,error,sizeof error));assert(gfxdump_mkdirs(dir,error,sizeof error));
    snprintf(path,sizeof path,"%s/sprite.png",dir);
    uint8_t px[35];for(int i=0;i<35;i++)px[i]=(uint8_t)(i%17);
    SDL_Color pal[256];for(int i=0;i<256;i++)pal[i]=(SDL_Color){(uint8_t)i,(uint8_t)(255-i),42,255};
    assert(gfxdump_write_png(path,7,5,px,0,pal,error,sizeof error));
    SDL_Surface*s=gfxdump_load_png(path);assert(s&&s->w==7&&s->h==5);SDL_DestroySurface(s);
    struct stat st;assert(stat(path,&st)==0&&(st.st_mode&0777)==0644);assert(stat(dir,&st)==0&&(st.st_mode&0777)==0755);
    char report[1100];snprintf(report,sizeof report,"%s/report.txt",dir);
    FILE*f=gfxdump_fopen(report,"w");assert(f);assert(fputs("kernel-backed report\n",f)>=0&&fclose(f)==0);
    f=gfxdump_fopen(report,"rb");assert(f);char line[80];assert(fgets(line,sizeof line,f)&&!strcmp(line,"kernel-backed report\n"));assert(fclose(f)==0);
    denied_open=1;assert(!gfxdump_write_png(path,7,5,px,0,pal,error,sizeof error));assert(strstr(error,"Permission denied"));denied_open=0;
    fail_sync=1;assert(!gfxdump_write_png(path,7,5,px,0,pal,error,sizeof error));assert(strstr(error,"Input/output error"));fail_sync=0;
    assert(gfxdump_verify_png(path,7,5,error,sizeof error));char temp[1200];snprintf(temp,sizeof temp,"%s.tmp",path);assert(access(temp,F_OK)!=0);
    assert(opens>6&&mkdirs>2&&syncs==2&&renames==1);
    SDL_Quit();puts("PASS: PS5 kernel adapter writes/reads/decodes PNG and report while ordinary pathname APIs are denied; native errors, sync failure, permissions and preservation checked");return 0;
}
