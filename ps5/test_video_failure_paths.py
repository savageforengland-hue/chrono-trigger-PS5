#!/usr/bin/env python3
"""Exercise the pinned PS5 framebuffer creation function with failed native APIs.

Uses the actual function source, not a duplicate algorithm; mocked native
boundaries verify success/failure and cleanup control flow on a Linux host.
"""
import argparse
from pathlib import Path
import subprocess,tempfile
PREFIX=r'''
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char Uint8;
#define SDL_max(a,b) ((a)>(b)?(a):(b))
#define SDL_zero(a) memset(&(a),0,sizeof(a))
#define SDL_PIXELFORMAT_ABGR8888 1
#define PS5_SOFTWARE_BUFFER_COUNT 2
#define PS5_SOFTWARE_MEMORY_SIZE (64u*1024u*1024u)
#define PS5_SCANOUT_TILE_PIXELS 16384u
typedef struct {void *data;} VideoBuffer;
typedef struct {int dummy;} PS5_VideoAttr;
typedef struct {int handle,evt_queue;size_t memsize;long paddr;bool mapped;VideoBuffer vbuf[2];void *surface;} PS5_DeviceData;
typedef struct {void *internal;} SDL_VideoDevice;
static int failure,cleanups;static void *allocation;
static char error_message[512];
static void SDL_Log(const char *format,...){(void)format;}
static bool SDL_SetError(const char *format,...){va_list args;va_start(args,format);vsnprintf(error_message,sizeof error_message,format,args);va_end(args);return false;}
static void PS5_DestroyPresentation(SDL_VideoDevice *device){
 PS5_DeviceData *p=device->internal;++cleanups;
 free(allocation);allocation=NULL;p->surface=NULL;p->mapped=false;p->handle=-1;
}
static int sceVideoOutOpen(int a,int b,int c,const void *d){(void)a;(void)b;(void)c;(void)d;return failure==1?-1:42;}
static int sceKernelAllocateMainDirectMemory(size_t bytes,size_t alignment,int type,long *paddr){
 (void)bytes;(void)alignment;(void)type;*paddr=10;return failure==2?-1:0;
}
static int sceKernelMapDirectMemory(void **mapped,size_t bytes,int protection,int flags,long physical,size_t alignment){
 (void)protection;(void)flags;(void)physical;(void)alignment;
 if(failure==3){return -1;}
 allocation=malloc(bytes);assert(allocation);*mapped=allocation;return 0;
}
static int sceKernelCreateEqueue(int *queue,const char *name){(void)name;*queue=7;return failure==4?-1:0;}
static int sceVideoOutAddFlipEvent(int queue,int handle,void *data){(void)queue;(void)handle;(void)data;return failure==5?-1:0;}
static int sceVideoOutSetFlipRate(int handle,int rate){(void)handle;(void)rate;return failure==6?-1:0;}
static void sceVideoOutSetBufferAttribute2(PS5_VideoAttr *attribute,uint64_t format,int tiling,uint32_t w,uint32_t h,int a,int b,int c){(void)attribute;(void)format;(void)tiling;(void)w;(void)h;(void)a;(void)b;(void)c;}
static int sceVideoOutRegisterBuffers2(int handle,int a,int b,VideoBuffer *buffers,int count,PS5_VideoAttr *attribute,int c,void *d){(void)handle;(void)a;(void)b;(void)buffers;(void)count;(void)attribute;(void)c;(void)d;return failure==7?-1:0;}
static void *SDL_CreateSurfaceZeroed(int w,int h,int format){(void)w;(void)h;(void)format;return failure==8?NULL:(void *)(uintptr_t)1;}
'''
SUFFIX=r'''
int main(void){
 for(failure=0;failure<=8;++failure){
  PS5_DeviceData data={0};SDL_VideoDevice device={&data};cleanups=0;
  error_message[0]=0;
  bool okay=PS5_CreatePresentation(&device,16,16);
  assert(okay==(failure==0));
  const char *names[]={"", "sceVideoOutOpen", "sceKernelAllocateMainDirectMemory", "sceKernelMapDirectMemory", "sceKernelCreateEqueue", "sceVideoOutAddFlipEvent", "sceVideoOutSetFlipRate", "sceVideoOutRegisterBuffers2"};
  if(failure>0 && failure<8){assert(strstr(error_message,names[failure]));assert(strstr(error_message,"0xffffffff"));}
  if(failure==1)assert(cleanups==0);
  else if(failure)assert(cleanups==1);
  if(!failure)PS5_DestroyPresentation(&device);
  assert(!allocation);printf("PASS: framebuffer creation native-stage %d\n",failure);
 }
 const int modes[][2]={{1920,1080},{2560,1440},{3840,2160}};
 failure=0;
 for(unsigned i=0;i<3;i++) {
  PS5_DeviceData data={0};SDL_VideoDevice device={&data};
  assert(PS5_CreatePresentation(&device,modes[i][0],modes[i][1]));
  const size_t bytes=PS5_TiledFrameBytes(modes[i][0],modes[i][1]);
  assert(data.memsize/2>=bytes && data.memsize%0x40000u==0);
  assert((size_t)((char*)data.vbuf[1].data-(char*)data.vbuf[0].data)==data.memsize/2);
  PS5_DestroyPresentation(&device);
  printf("PASS: padded scanout allocation %dx%d\n",modes[i][0],modes[i][1]);
 }
 return 0;
}
'''
def function_source(path,signature='static bool PS5_CreatePresentation('):
 text=path.read_text();start=text.index(signature)
 body=text.index('{',start);depth=0
 for index in range(body,len(text)):
  if text[index]=='{':depth+=1
  elif text[index]=='}':
   depth-=1
   if not depth:return text[start:index+1]
 raise ValueError('Function end not found')
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',required=True,type=Path);a=p.parse_args()
 video=a.sdl_source/'src/video/ps5/SDL_ps5video.c'
 function=function_source(video,'static size_t PS5_TiledFrameBytes(')+'\n'+function_source(video)
 with tempfile.TemporaryDirectory() as tmp:
  root=Path(tmp);source=root/'test.c';source.write_text(PREFIX+'\n'+function+'\n'+SUFFIX)
  exe=root/'test';subprocess.run(['cc','-std=c11','-O1','-Wall','-Wextra',str(source),'-o',str(exe)],check=True)
  subprocess.run([str(exe)],check=True,timeout=15)
 print('9 error-path cases and 3 padded-allocation profiles passed. Native PS5 video behavior remains untested.')
if __name__=='__main__':main()
