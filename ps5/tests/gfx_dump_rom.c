/* SPDX-License-Identifier: GPL-3.0-or-later
 * Optional full own-ROM dump. No ROM/art bytes are part of the test source. */
#include "runtime/rt.h"
#include "runtime/gfx_dump.h"
#include <assert.h>
#include <SDL3/SDL.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
RtRom rt_rom;static char root[1024];
const char*rt_data_dir(void){return root;}
void rt_log(const char*fmt,...){va_list a;va_start(a,fmt);vfprintf(stderr,fmt,a);fputc('\n',stderr);va_end(a);}
int main(int argc,char**argv){
    assert(argc==3||argc==4);int repair=argc==4&&!strcmp(argv[3],"repair"),check=argc==4&&(!strcmp(argv[3],"check")||!strcmp(argv[3],"check-fail"));int expect_fail=argc==4&&(!strcmp(argv[3],"expect-fail")||!strcmp(argv[3],"check-fail"));umask(0077);snprintf(root,sizeof root,"%s/",argv[2]);
    FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);long n=ftell(f);assert(n>512&&n<=512*1024*1024L);rewind(f);
    rt_rom.size=(size_t)n;rt_rom.data=malloc((size_t)n);assert(rt_rom.data);assert(fread(rt_rom.data,1,(size_t)n,f)==(size_t)n);fclose(f);
    uint8_t*copy=malloc((size_t)n);assert(copy);memcpy(copy,rt_rom.data,(size_t)n);
    assert(SDL_Init(0));if(repair)gfxdump_repair_start();else if(check)gfxdump_check_start();else gfxdump_start();Uint64 begin=SDL_GetTicks(),log_at=begin;
    while(gfxdump_running()){
        assert(SDL_GetTicks()-begin<300000);SDL_Delay(10);
        if(SDL_GetTicks()-log_at>=3000){int d,t;float p;gfxdump_progress(&d,&t,&p);fprintf(stderr,"Dump progress %d/%d %.1f%%\n",d,t,p*100);log_at=SDL_GetTicks();}
    }
    int images=-1;assert(gfxdump_finished(&images));assert(!gfxdump_finished(&images));
    if(expect_fail){assert(images==0&&gfxdump_errors()>0&&gfxdump_error()[0]);}
    else assert(images>0&&gfxdump_errors()==0);
    assert(gfxdump_repairing()==repair);assert(gfxdump_checking()==check);if(check&&!expect_fail)assert(images==1);
    assert(!memcmp(copy,rt_rom.data,(size_t)n));printf("RESULT images=%d errors=%d\n",images,gfxdump_errors());
    free(copy);free(rt_rom.data);SDL_Quit();return 0;
}
