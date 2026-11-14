#!/usr/bin/env python3
"""Run the actual SDL PS5 uploader against an independent native tile decoder.

Checks every pixel and padded tile position, pitch, guards, and partial worker
launches. No ROM or game images are used. Optional legacy source must fail.
"""
import argparse
from pathlib import Path
import re,resource,subprocess,tempfile

PREFIX=r'''
#include <assert.h>
#include <emmintrin.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL_ps5tilemap.inc"
static int fail_after=-1, launches;
static int create_thread(pthread_t *thread,const pthread_attr_t *attr,void *(*entry)(void*),void *arg) {
 if(fail_after>=0 && launches++==fail_after)return EAGAIN;
 return pthread_create(thread,attr,entry,arg);
}
#define pthread_create create_thread
'''
HARNESS=r'''
/* Hardware address oracle: independent of the upload table and traversal. */
static size_t native_address(unsigned x,unsigned y,unsigned width) {
 unsigned low=((y<<4)&0x70u)^((y<<5)&0xf00u)^((y<<9)&0x1000u)^((y<<8)&0x4000u)^
              ((x<<2)&0xcu)^((x<<5)&0x380u)^((x<<4)&0x400u)^((x<<6)&0x800u)^((x<<9)&0xa000u);
 return ((((size_t)(y>>7)*((width+127u)>>7)+(x>>7))<<16)+low)/4;
}
static int frame(int width,int height,int fail) {
 const unsigned columns=((unsigned)width+127u)/128u,rows=((unsigned)height+127u)/128u;
 const size_t frame_bytes=(size_t)columns*rows*65536u;
#ifdef LEGACY
 const size_t source_pitch=(size_t)width;
#else
 const size_t source_pitch=(size_t)width+13u;
 assert(PS5_TiledFrameBytes(width,height)==frame_bytes);
#endif
 uint32_t *source=malloc(source_pitch*(size_t)height*4);assert(source);
 memset(source,0xee,source_pitch*(size_t)height*4);
 for(int y=0;y<height;y++)for(int x=0;x<width;x++)source[(size_t)y*source_pitch+x]=0xff000000u+(unsigned)y*(unsigned)width+(unsigned)x;
 unsigned char *raw=NULL;assert(!posix_memalign((void**)&raw,64,frame_bytes+128));
 memset(raw,0xa5,frame_bytes+128);uint32_t *destination=(uint32_t*)(raw+64);
 fail_after=fail;launches=0;
#ifdef LEGACY
 PS5_DrawPixelsAsTiles(source,destination,width,height);
#else
 PS5_DrawPixelsAsTiles(source,destination,width,height,(int)source_pitch*4);
#endif
 for(unsigned y=0;y<rows*128u;y++)for(unsigned x=0;x<columns*128u;x++) {
  const uint32_t expected=x<(unsigned)width && y<(unsigned)height ? 0xff000000u+y*(unsigned)width+x : 0;
  const uint32_t actual=destination[native_address(x,y,(unsigned)width)];
  if(actual!=expected) {
   fprintf(stderr,"pixel mismatch %dx%d at %u,%u: %08x expected %08x\n",width,height,x,y,actual,expected);
   free(source);free(raw);return 1;
  }
 }
 for(unsigned i=0;i<64;i++)assert(raw[i]==0xa5 && raw[64+frame_bytes+i]==0xa5);
 free(source);free(raw);printf("PASS: %dx%d, worker failure after %d, full native decode/padding/pitch/guards\n",width,height,fail);
 return 0;
}
int main(void) {
 const int sizes[][2]={{3840,2160},{1920,1080},{2560,1440},{129,129},{1,1}};
 const int failures[]={-1,0,5};
 for(unsigned i=0;i<5;i++)for(unsigned j=0;j<3;j++)if(frame(sizes[i][0],sizes[i][1],failures[j]))return 1;
 return 0;
}
'''

def extract(path):
    source=path.read_text()
    return source[source.index('#define PS5_THREAD_COUNT'):source.index('static void PS5_DestroyPresentation(')]

def main():
    resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--sdl-source',type=Path,required=True)
    p.add_argument('--legacy-source',type=Path)
    a=p.parse_args();video=a.sdl_source/'src/video/ps5'
    header=(video/'SDL_ps5video.h').read_text()
    chunk=re.search(r'typedef struct PS5_DrawChunk \{.*?\} PS5_DrawChunk;',header,re.S).group()
    with tempfile.TemporaryDirectory() as tmp:
        root=Path(tmp)
        for legacy in (False,True) if a.legacy_source else (False,):
            if legacy:
                chunk='typedef struct {uint32_t *src,*dst;int frame_width,frame_height;size_t src_start,src_end;} PS5_DrawChunk;'
            source=root/('legacy.c' if legacy else 'current.c')
            source.write_text(('#define LEGACY 1\n' if legacy else '')+PREFIX+chunk+'\n'+extract(a.legacy_source if legacy else video/'SDL_ps5video.c')+HARNESS)
            exe=source.with_suffix('')
            command=['cc','-D_POSIX_C_SOURCE=200809L','-std=c11','-O2','-pthread','-I',str(video),str(source),'-o',str(exe)]
            subprocess.run(command,check=True)
            result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=45)
            if legacy:
                assert result.returncode!=0 and 'pixel mismatch 3840x2160' in result.stderr,result
                print('PASS: previous uploader rejected by native decoder:',result.stderr.strip())
            else:
                print(result.stdout,end='');assert result.returncode==0,result.stderr
    print('15 scanout scenarios passed; actual display behavior still needs a PS5 retest.')

if __name__=='__main__':main()
