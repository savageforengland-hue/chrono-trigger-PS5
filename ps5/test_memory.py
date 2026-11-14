#!/usr/bin/env python3
"""Exercise the vendored direct-memory allocator and the real preflight.

Uses the pinned platform's host kernel fixture, not PS5 hardware. Each failure
scenario runs in a new process so allocator initialization is tested afresh.
"""
import os
from pathlib import Path
import subprocess
import tempfile

HARNESS = r'''
#include "memory.h"
#include "ps5platform/heap.h"
#include "tests/host_kernel.h"
#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int libc_denied, preflight_failure;
static char last_stage[256];
void *__real_malloc(size_t n) { return libc_denied ? (errno=ENOMEM, (void*)0) : malloc(n); }
void *__real_calloc(size_t n,size_t s) { return libc_denied ? (errno=ENOMEM, (void*)0) : calloc(n,s); }
void *__real_realloc(void *p,size_t n) { return libc_denied ? (errno=ENOMEM, (void*)0) : realloc(p,n); }
void __real_free(void *p) { free(p); }
int __real_posix_memalign(void **p,size_t a,size_t n) { return libc_denied ? ENOMEM : posix_memalign(p,a,n); }
size_t __real_malloc_usable_size(const void *p) { return malloc_usable_size((void*)p); }
void egg_ps5_diag_stage(int stage,const char *text) { assert(stage==35);snprintf(last_stage,sizeof last_stage,"%s",text); }
void egg_ps5_diag_message(const char *text) { assert(strstr(text,"direct-heap base=")); }
void egg_ps5_diag_failure(const char *stage,const char *text) {
 assert(!strcmp(stage,"memory preflight"));assert(strstr(text,"failed; errno="));preflight_failure++;
}
void *__wrap_malloc(size_t);
void *__wrap_calloc(size_t,size_t);
void *__wrap_realloc(void*,size_t);
void *__wrap_reallocarray(void*,size_t,size_t);
void __wrap_free(void*);
int __wrap_posix_memalign(void**,size_t,size_t);
size_t __wrap_malloc_usable_size(const void*);
static void *handed[256];
static void *worker(void *arg) {
 unsigned id=(unsigned)(uintptr_t)arg;
 for(unsigned i=id;i<256;i+=4) {
  unsigned char *p=__wrap_realloc(handed[i],4096);
  assert(p && p[0]==(unsigned char)i && ps5_heap_owns(p));__wrap_free(p);
 }
 for(unsigned i=0;i<2000;i++) {
  size_t n=32+(i*37u)%70000;unsigned char *p=__wrap_malloc(n);
  assert(p && ps5_heap_owns(p));p[0]=1;p[n-1]=2;__wrap_free(p);
 }
 return NULL;
}
int main(int argc,char **argv) {
 assert(argc==2);const char *mode=argv[1];
 if(!strcmp(mode,"reserve"))host_fail(HOST_CALL_RESERVE,-1);
 if(!strcmp(mode,"allocate") || !strcmp(mode,"exhausted"))host_fail(HOST_CALL_ALLOCATE,-1);
 if(!strcmp(mode,"map"))host_fail(HOST_CALL_MAP,-1);
 if(!strcmp(mode,"exhausted"))libc_denied=1;
 if(strcmp(mode,"success")) {
  bool ok=egg_ps5_memory_prepare();
  if(!strcmp(mode,"exhausted"))assert(!ok && preflight_failure==1);
  else { assert(ok && !preflight_failure && strstr(last_stage,"libc fallback")); }
  assert(host_direct_allocations()==0);
  printf("PASS: %s failure handled, no direct allocation leaked\n",mode);return 0;
 }
 assert(egg_ps5_memory_prepare() && !preflight_failure);
 assert(strstr(last_stage,"application direct-memory heap"));
 unsigned char *p=__wrap_malloc(24);assert(p && ps5_heap_owns(p) && (uintptr_t)p%32==0);
 memset(p,0x75,24);unsigned char *grown=__wrap_realloc(p,100000);
 assert(grown && ps5_heap_owns(grown));for(unsigned i=0;i<24;i++)assert(grown[i]==0x75);
 assert(__wrap_malloc_usable_size(grown)>=100000);__wrap_free(grown);
 unsigned char *zero=__wrap_calloc(128,16);assert(zero);for(unsigned i=0;i<2048;i++)assert(zero[i]==0);__wrap_free(zero);
 assert(__wrap_calloc(SIZE_MAX,2)==NULL);
 p=__wrap_malloc(32);p[0]=0x45;errno=0;
 assert(__wrap_reallocarray(p,SIZE_MAX,2)==NULL && errno==ENOMEM && p[0]==0x45);__wrap_free(p);
 for(size_t alignment=8;alignment<=65536;alignment*=2) {
  void *aligned=NULL;assert(__wrap_posix_memalign(&aligned,alignment,100)==0);
  assert(aligned && (uintptr_t)aligned%alignment==0 && ps5_heap_owns(aligned));__wrap_free(aligned);
 }
 void *unaligned=(void*)1;assert(__wrap_posix_memalign(&unaligned,24,100)==EINVAL && unaligned==(void*)1);
 struct ps5_heap_stats before,after;ps5_heap_stats(&before);
 const size_t rom_bytes=(size_t)128<<20;
 p=__wrap_malloc(rom_bytes);assert(p && ps5_heap_owns(p));p[0]=1;p[rom_bytes-1]=2;
 ps5_heap_stats(&after);assert(after.mapped_bytes>=before.mapped_bytes+rom_bytes);
 __wrap_free(p);ps5_heap_stats(&after);assert(after.mapped_bytes==before.mapped_bytes);
 char *foreign=strdup("foreign libc block");assert(foreign && !ps5_heap_owns(foreign));
 grown=__wrap_realloc(foreign,4096);assert(grown && !strcmp((char*)grown,"foreign libc block"));__wrap_free(grown);
 host_fail(HOST_CALL_ALLOCATE,-1);p=__wrap_malloc((size_t)64<<20);
 assert(p && !ps5_heap_owns(p));__wrap_free(p);host_fail(HOST_CALL_NONE,0);
 p=__wrap_malloc(64);p[0]=0x12;libc_denied=1;host_fail(HOST_CALL_ALLOCATE,-1);
 assert(__wrap_realloc(p,(size_t)128<<20)==NULL && p[0]==0x12);
 libc_denied=0;host_fail(HOST_CALL_NONE,0);__wrap_free(p);
 for(unsigned i=0;i<256;i++){handed[i]=__wrap_malloc(64);assert(handed[i]);((unsigned char*)handed[i])[0]=(unsigned char)i;}
 pthread_t threads[4];for(unsigned i=0;i<4;i++)assert(!pthread_create(&threads[i],NULL,worker,(void*)(uintptr_t)i));
 for(unsigned i=0;i<4;i++)assert(!pthread_join(threads[i],NULL));
 puts("PASS: direct heap preflight, alignment, calloc/overflow, preserved resize, 128MiB allocate/release, foreign pointers, fallback/exhaustion and four-thread cross-arena resize/free");
 return 0;
}
'''

def main():
    here=Path(__file__).resolve().parent
    with tempfile.TemporaryDirectory() as tmp:
        d=Path(tmp);h=d/'harness.c';h.write_text(HARNESS)
        cc=['cc','-D_GNU_SOURCE','-std=gnu11','-O1','-g','-pthread','-I',str(here),'-I',str(here/'compat')]
        memory=d/'memory.o'
        subprocess.run(cc+['-Dmalloc=__wrap_malloc','-Dcalloc=__wrap_calloc','-Drealloc=__wrap_realloc','-Dfree=__wrap_free','-c',str(here/'memory.c'),'-o',str(memory)],check=True)
        exe=d/'memory-test'
        subprocess.run(cc+[str(here/'compat'/p) for p in ['heap.c','heap_wrap.c','placement.c','dlmalloc/mspace.c','tests/host_kernel.c']]+[str(h),str(memory),'-o',str(exe)],check=True)
        for mode in ('success','reserve','allocate','map','exhausted'):
            subprocess.run([str(exe),mode],check=True,timeout=45)
    print('5 allocator integration scenarios passed using host kernel mocks; PS5 behavior is unverified.')

if __name__=='__main__':main()
