#!/usr/bin/env python3
"""Host tests for adapted directory, FP and shell-close behavior; not PS5 tests."""
from pathlib import Path
import subprocess
import tempfile

LIFECYCLE = r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int seen, mode;
void catchReturnFromMain(int);
void egg_ps5_diag_stage(int stage,const char *message) {
 assert(message);
 if(!seen){assert(stage==110);assert(strstr(message,"main status=7"));seen=1;}
 else {assert(stage==111);assert(strstr(message,"waiting for shell"));seen=2;}
}
int sceSystemServiceLoadExec(const char *name,const char *const *argv) {
 assert(seen==1 && !strcmp(name,"exit") && !argv);return mode?-1:0;
}
int sceKernelUsleep(unsigned microseconds) {
 assert(seen==2 && microseconds==100000);
 puts("shell-close API and asynchronous wait PASS");fflush(stdout);_Exit(0);
}
int main(int argc,char **argv){(void)argv;mode=argc>1;catchReturnFromMain(7);return 1;}
'''
FP = r'''
#include <assert.h>
#include <stdio.h>
void ps5_fp_ieee(void);
int main(void){unsigned bad=0x9fc0,actual;unsigned short x87;
 __asm__ volatile("ldmxcsr %0"::"m"(bad));ps5_fp_ieee();
 __asm__ volatile("stmxcsr %0":"=m"(actual));
 __asm__ volatile("fnstcw %0":"=m"(x87));
 assert(actual==0x1f80 && x87==0x37f);puts("native IEEE floating-point state PASS");return 0;}
'''


def main():
    here=Path(__file__).resolve().parent
    parser=__import__('argparse').ArgumentParser(description=__doc__)
    parser.add_argument('--sdl-source',required=True,type=Path)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory() as temporary:
        temp=Path(temporary)
        directory=temp/'directory'
        subprocess.run(['cc','-std=gnu11','-O1','-Wall','-Wextra','-I',str(here/'compat'),
                        '-DEGG_PS5_DIRECTORY_STREAMS_ONLY',str(here/'compat/directory.c'),
                        str(here/'compat/test_directory_records.c'),'-pthread',
                        '-Wl,--wrap=open','-Wl,--wrap=close','-o',str(directory)],check=True)
        subprocess.run([str(directory)],check=True)
        fp=temp/'fp.c';fp.write_text(FP)
        subprocess.run(['cc','-std=c11','-O2',str(here/'compat/fp.c'),str(fp),'-o',str(temp/'fp')],check=True)
        subprocess.run([str(temp/'fp')],check=True)
        lifecycle=temp/'lifecycle.c';lifecycle.write_text(LIFECYCLE)
        subprocess.run(['cc','-std=c11','-O2','-ffunction-sections','-fdata-sections',
                        '-I',str(args.sdl_source/'include'),str(here/'platform.c'),str(lifecycle),
                        '-Wl,--gc-sections','-o',str(temp/'lifecycle')],check=True)
        for mode in ([],['failed-request']):
            subprocess.run([str(temp/'lifecycle')]+mode,check=True,timeout=5)
    print('Host compatibility tests passed; native console behavior remains unverified.')


if __name__=='__main__':main()
