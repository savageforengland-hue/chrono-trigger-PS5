/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include "runtime/spu.c"
static Uint64 clock_ms=100;
static int queue_bytes,put_calls,logs,fail_put;
static char message[256];
Uint64 SDL_GetTicks(void){return clock_ms;}
int SDL_GetAudioStreamQueued(SDL_AudioStream*s){(void)s;return queue_bytes;}
bool SDL_PutAudioStreamData(SDL_AudioStream*s,const void*p,int n){(void)s;(void)p;assert(n>0);put_calls++;return !fail_put;}
void rt_log(const char*fmt,...){va_list args;va_start(args,fmt);vsnprintf(message,sizeof message,fmt,args);va_end(args);logs++;}
int main(void){
    float samples[960*2]={0};stream=(SDL_AudioStream*)1;
    output(samples,960);assert(put_calls==2&&!logs); /* initial silence + samples */
    queue_bytes=38400;clock_ms+=20;output(samples,960);assert(put_calls==3); /* 100ms */
    queue_bytes=96000;clock_ms+=20;output(samples,960);assert(put_calls==3); /* cap drops */
    queue_bytes=-1;clock_ms+=20;output(samples,960);assert(put_calls==3); /* query failure */
    queue_bytes=0;fail_put=1;clock_ms+=20;output(samples,960);assert(put_calls==5);
    clock_ms=5100;queue_bytes=38400;fail_put=0;output(samples,960);
    assert(logs==1&&strstr(message,"empty=1 dropped=1 errors=3")&&strstr(message,"max_producer_gap_ms=4920"));
    clock_ms+=20;output(samples,960);assert(logs==1);
    clock_ms=10100;output(samples,960);assert(logs==2&&strstr(message,"empty=0 dropped=0 errors=0"));
    fprintf(stderr,"PASS: actual audio output startup padding, queue cap, error handling, five-second diagnostics and counter reset\n");
}
