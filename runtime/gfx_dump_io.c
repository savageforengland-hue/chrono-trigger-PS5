/* SPDX-License-Identifier: GPL-3.0-or-later
 * Dump-only filesystem adapter. PS5 uses the explicit kernel path APIs used
 * by the persistent diagnostic log; SDL encodes to memory, never a pathname.
 * No global libc/SDL hooks and no changes to saves or installed mod loading. */
#define _POSIX_C_SOURCE 200809L
#include "gfx_dump_io.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#ifdef EGG_PS5
extern int sceKernelOpen(const char*,int,unsigned);
extern int sceKernelClose(int);
extern int sceKernelMkdir(const char*,unsigned);
extern int sceKernelChmod(const char*,unsigned);
extern int sceKernelRename(const char*,const char*);
extern int sceKernelUnlink(const char*);
extern int sceKernelFstat(int,struct stat*);
extern int sceKernelFsync(int);
static int kernel_result(int result){
    if(result<0){errno=(int)((unsigned)result&0xffffu);return -1;}return result;
}
static int make_dir(const char*path){return kernel_result(sceKernelMkdir(path,0755));}
int gfxdump_chmod(const char*path,unsigned mode){return kernel_result(sceKernelChmod(path,mode));}
static int publish(const char*from,const char*to){return kernel_result(sceKernelRename(from,to));}
static int remove_temp(const char*path){return kernel_result(sceKernelUnlink(path));}
static int sync_file(FILE*f){return kernel_result(sceKernelFsync(fileno(f)));}
FILE*gfxdump_fopen(const char*path,const char*mode){
    int flags=mode[0]=='w'?O_WRONLY|O_CREAT|O_TRUNC:mode[0]=='r'?O_RDONLY:-1;
    if(flags<0){errno=EINVAL;return NULL;}
    int fd=kernel_result(sceKernelOpen(path,flags,0644));if(fd<0)return NULL;
    FILE*f=fdopen(fd,mode);
    if(!f){int saved=errno;sceKernelClose(fd);errno=saved;}return f;
}
static int check_directory(const char*path){
    int fd=kernel_result(sceKernelOpen(path,O_RDONLY|O_DIRECTORY,0));if(fd<0)return 0;
    struct stat st;int result=kernel_result(sceKernelFstat(fd,&st));int saved=errno;
    if(result==0&&!S_ISDIR(st.st_mode)){result=-1;saved=ENOTDIR;}
    if(kernel_result(sceKernelClose(fd))<0&&result==0)return 0;
    errno=saved;return result==0;
}
#else
static int make_dir(const char*path){return mkdir(path,0755);}
int gfxdump_chmod(const char*path,unsigned mode){return chmod(path,(mode_t)mode);}
static int publish(const char*from,const char*to){return rename(from,to);}
static int remove_temp(const char*path){return unlink(path);}
static int sync_file(FILE*f){return fsync(fileno(f));}
FILE*gfxdump_fopen(const char*path,const char*mode){return fopen(path,mode);}
static int check_directory(const char*path){
    DIR*d=opendir(path);if(!d)return 0;return closedir(d)==0;
}
#endif

static int error_text(char*out,size_t n,const char*action,const char*path){
    int e=errno;snprintf(out,n,"%s %s: %s",action,path,strerror(e?e:EIO));return 0;
}
int gfxdump_mkdirs(const char*path,char*error,size_t capacity){
    char buf[1024];size_t n=strlen(path);
    if(!n||n>=sizeof buf){errno=ENAMETOOLONG;return error_text(error,capacity,"Directory",path);}
    if(make_dir(path)==0){if(gfxdump_chmod(path,0755)==0)return 1;return error_text(error,capacity,"Directory permissions",path);}
    if(errno==EEXIST){
        if(!check_directory(path))return error_text(error,capacity,"Open directory",path);
        if(gfxdump_chmod(path,0755)!=0)return error_text(error,capacity,"Directory permissions",path);
        return 1;
    }
    if(errno!=ENOENT)return error_text(error,capacity,"Create directory",path);
    memcpy(buf,path,n+1);
    for(size_t i=1;i<=n;i++)if(buf[i]=='/'||!buf[i]){
        char saved=buf[i];buf[i]=0;
        if(make_dir(buf)==0){
            if(gfxdump_chmod(buf,0755)!=0)return error_text(error,capacity,"Directory permissions",buf);
        }else{
            if(errno!=EEXIST)return error_text(error,capacity,"Create directory",buf);
            if(!check_directory(buf))return error_text(error,capacity,"Open directory",buf);
        }buf[i]=saved;
    }
    if(gfxdump_chmod(path,0755)!=0)return error_text(error,capacity,"Directory permissions",path);
    return 1;
}
static void be32(uint8_t*p,uint32_t n){p[0]=(uint8_t)(n>>24);p[1]=(uint8_t)(n>>16);p[2]=(uint8_t)(n>>8);p[3]=(uint8_t)n;}
int gfxdump_verify_png(const char*path,int w,int h,char*error,size_t capacity){
    static const uint8_t signature[8]={137,80,78,71,13,10,26,10};
    static const uint8_t iend[12]={0,0,0,0,73,69,78,68,174,66,96,130};
    uint8_t head[33],tail[12],width[4],height[4];be32(width,(uint32_t)w);be32(height,(uint32_t)h);
    FILE*f=gfxdump_fopen(path,"rb");if(!f)return error_text(error,capacity,"Verify PNG",path);
    int okay=fread(head,1,sizeof head,f)==sizeof head && !memcmp(head,signature,8) &&
        !memcmp(head+12,"IHDR",4) && (w==0||!memcmp(head+16,width,4)) && (h==0||!memcmp(head+20,height,4)) &&
        head[24]==8 && head[25]==3 && fseek(f,-12,SEEK_END)==0 &&
        fread(tail,1,sizeof tail,f)==sizeof tail && !memcmp(tail,iend,12);
    if(fclose(f))okay=0;
    if(okay)return 1;
    errno=EIO;return error_text(error,capacity,"Invalid or incomplete PNG",path);
}
int gfxdump_write_png(const char*path,int w,int h,const uint8_t*pixels,int bpp8,const SDL_Color colors[256],char*error,size_t capacity){
    if(!pixels||!colors||w<=0||h<=0||(size_t)h>SIZE_MAX/((size_t)w+1)){
        errno=EINVAL;return error_text(error,capacity,"PNG dimensions",path);
    }
    char temp[1100];if(snprintf(temp,sizeof temp,"%s.tmp",path)>=(int)sizeof temp){errno=ENAMETOOLONG;return error_text(error,capacity,"PNG path",path);}
    SDL_Surface*s=SDL_CreateSurfaceFrom(w,h,SDL_PIXELFORMAT_INDEX8,(void*)pixels,w);
    SDL_Palette*p=s?SDL_CreateSurfacePalette(s):NULL;SDL_Color c[256];
    for(int i=0;i<256;i++){c[i]=colors[i];c[i].a=(bpp8?i==0:(i&15)==0)?0:255;}
    SDL_IOStream*encoded=SDL_IOFromDynamicMem();
    errno=0;int okay=p&&encoded&&SDL_SetPaletteColors(p,c,0,256)&&SDL_SavePNG_IO(s,encoded,false);
    SDL_DestroySurface(s);int e=errno;
    if(!okay){if(encoded)SDL_CloseIO(encoded);errno=e?e:EIO;return error_text(error,capacity,"Encode PNG",path);}
    Sint64 length=SDL_GetIOSize(encoded);
    const uint8_t*bytes=SDL_GetPointerProperty(SDL_GetIOProperties(encoded),SDL_PROP_IOSTREAM_DYNAMIC_MEMORY_POINTER,NULL);
    FILE*f=length>0&&bytes?gfxdump_fopen(temp,"wb"):NULL;
    okay=f!=NULL;
    if(f){
        okay=fwrite(bytes,1,(size_t)length,f)==(size_t)length&&fflush(f)==0&&sync_file(f)==0;e=errno;
        if(fclose(f)){okay=0;e=errno;}
    }else e=errno;
    if(!okay){SDL_CloseIO(encoded);remove_temp(temp);errno=e?e:EIO;return error_text(error,capacity,"Write/sync PNG",path);}
    /* Read the entire file back through the same explicit kernel route. */
    f=gfxdump_fopen(temp,"rb");okay=f!=NULL;
    if(f){
        uint8_t block[4096];size_t at=0;
        while(okay&&at<(size_t)length){size_t n=SDL_min(sizeof block,(size_t)length-at);okay=fread(block,1,n,f)==n&&!memcmp(block,bytes+at,n);at+=n;}
        if(fclose(f))okay=0;
    }
    SDL_CloseIO(encoded);
    if(!okay){remove_temp(temp);errno=EIO;return error_text(error,capacity,"PNG byte readback",path);}
    if(gfxdump_chmod(temp,0644)!=0){e=errno;remove_temp(temp);errno=e;return error_text(error,capacity,"PNG permissions",path);}
    if(!gfxdump_verify_png(temp,w,h,error,capacity)){remove_temp(temp);return 0;}
    if(publish(temp,path)!=0){e=errno;remove_temp(temp);errno=e;return error_text(error,capacity,"Publish PNG",path);}
    return gfxdump_verify_png(path,w,h,error,capacity);
}

/* Decode using bytes read via the dump adapter, not SDL_LoadPNG(path). */
SDL_Surface*gfxdump_load_png(const char*path){
    FILE*f=gfxdump_fopen(path,"rb");if(!f)return NULL;
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return NULL;}
    long size=ftell(f);
    if(size<=0||size>128*1024*1024||fseek(f,0,SEEK_SET)!=0){fclose(f);return NULL;}
    uint8_t*bytes=malloc((size_t)size);int okay=bytes&&fread(bytes,1,(size_t)size,f)==(size_t)size;
    if(fclose(f))okay=0;
    SDL_Surface*surface=NULL;
    if(okay){SDL_IOStream*io=SDL_IOFromConstMem(bytes,(size_t)size);if(io)surface=SDL_LoadPNG_IO(io,true);}
    free(bytes);return surface;
}
