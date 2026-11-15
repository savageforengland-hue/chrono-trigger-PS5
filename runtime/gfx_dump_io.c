/* SPDX-License-Identifier: GPL-3.0-or-later
 * Checked native paths and FTP-readable permissions around SDL's indexed
 * encoder. The original encoder is retained: the reported failure was FTP
 * directory access, not missing image support. */
#include "gfx_dump_io.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static int error_text(char*out,size_t n,const char*action,const char*path){
    int e=errno;snprintf(out,n,"%s %s: %s",action,path,strerror(e?e:EIO));return 0;
}
int gfxdump_mkdirs(const char*path,char*error,size_t capacity){
    char buf[1024];size_t n=strlen(path);
    if(!n||n>=sizeof buf){errno=ENAMETOOLONG;return error_text(error,capacity,"Directory",path);}
    if(mkdir(path,0755)==0){if(chmod(path,0755)==0)return 1;return error_text(error,capacity,"Directory permissions",path);}
    if(errno==EEXIST){
        DIR*d=opendir(path);if(!d)return error_text(error,capacity,"Open directory",path);
        if(closedir(d)!=0)return error_text(error,capacity,"Close directory",path);
        if(chmod(path,0755)!=0)return error_text(error,capacity,"Directory permissions",path);
        return 1;
    }
    if(errno!=ENOENT)return error_text(error,capacity,"Create directory",path);
    memcpy(buf,path,n+1);
    for(size_t i=1;i<=n;i++)if(buf[i]=='/'||!buf[i]){
        char saved=buf[i];buf[i]=0;
        if(mkdir(buf,0755)==0){
            if(chmod(buf,0755)!=0)return error_text(error,capacity,"Directory permissions",buf);
        }else{
            if(errno!=EEXIST)return error_text(error,capacity,"Create directory",buf);
            DIR*d=opendir(buf);if(!d)return error_text(error,capacity,"Open directory",buf);
            if(closedir(d)!=0)return error_text(error,capacity,"Close directory",buf);
        }buf[i]=saved;
    }
    if(chmod(path,0755)!=0)return error_text(error,capacity,"Directory permissions",path);
    return 1;
}
static void be32(uint8_t*p,uint32_t n){p[0]=(uint8_t)(n>>24);p[1]=(uint8_t)(n>>16);p[2]=(uint8_t)(n>>8);p[3]=(uint8_t)n;}
int gfxdump_verify_png(const char*path,int w,int h,char*error,size_t capacity){
    static const uint8_t signature[8]={137,80,78,71,13,10,26,10};
    static const uint8_t iend[12]={0,0,0,0,73,69,78,68,174,66,96,130};
    uint8_t head[33],tail[12],width[4],height[4];be32(width,(uint32_t)w);be32(height,(uint32_t)h);
    FILE*f=fopen(path,"rb");if(!f)return error_text(error,capacity,"Verify PNG",path);
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
    errno=0;int okay=p&&SDL_SetPaletteColors(p,c,0,256)&&SDL_SavePNG(s,temp);int e=errno;
    SDL_DestroySurface(s);
    if(!okay){unlink(temp);errno=e?e:EIO;return error_text(error,capacity,"Save PNG",path);}
    if(chmod(temp,0644)!=0){e=errno;unlink(temp);errno=e;return error_text(error,capacity,"PNG permissions",path);}
    if(!gfxdump_verify_png(temp,w,h,error,capacity)){unlink(temp);return 0;}
    if(rename(temp,path)!=0){e=errno;unlink(temp);errno=e;return error_text(error,capacity,"Publish PNG",path);}
    return gfxdump_verify_png(path,w,h,error,capacity);
}
