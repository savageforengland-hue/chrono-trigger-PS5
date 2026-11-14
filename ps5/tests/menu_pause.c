/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdio.h>
#include "runtime/sched.c"
static Uint64 fake_ns=1000000000;
static int paused=4;
Uint64 SDL_GetTicksNS(void){return fake_ns;}
int SDL_GetAtomicInt(SDL_AtomicInt*a){return a->value;}
int SDL_SetAtomicInt(SDL_AtomicInt*a,int v){int old=a->value;a->value=v;return old;}
void SDL_Delay(Uint32 ms){fake_ns+=(Uint64)ms*1000000;}
int sysmenu_pauses_game(void){return paused-->0;}
int main(void){
 base_ns=1234;base_cycles=4567;rt_frame_count=90;
 park_on_menu();assert(base_ns==1234+15000000&&base_cycles==4567&&rt_frame_count==90);
 Uint64 before=base_ns;park_on_menu();assert(base_ns==before);
 puts("PASS: host menu parks game timing, preserves frame/cycle state, rebases exactly the paused interval and resumes without catch-up");
}
