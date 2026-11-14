#!/usr/bin/env python3
"""Host test of the actual no-libc native entry's notifications and log writes."""
from pathlib import Path
import subprocess,tempfile
HARNESS=r'''
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void boot_check_entry(void *,void *);
static const char *directory,*mode;
int sceKernelOpen(const char *path,int flags,unsigned permissions){
 assert(!strcmp(path,"/data/eggoftime-boot-check.log"));
 if(!strcmp(mode,"denied"))return -13;
 char local[1024];snprintf(local,sizeof local,"%s/probe.log",directory);
 return open(local,flags,permissions);
}
long sceKernelWrite(int fd,const void *data,size_t length){return write(fd,data,length);}
int sceKernelFsync(int fd){return fsync(fd);}
int sceKernelSendNotificationRequest(unsigned device,void *request,size_t size,unsigned blocking){
 (void)device;(void)size;(void)blocking;
 const char *message=(char *)request+45;assert(strstr(message,"native entry reached"));
 if(!strcmp(mode,"denied"))assert(strstr(message,"could not be opened"));
 return !strcmp(mode,"notify-failed")?-1:0;
}
int sceSystemServiceHideSplashScreen(void){return 0;}
int sceKernelUsleep(unsigned microseconds){assert(microseconds==100000);_Exit(0);}
int main(int argc,char **argv){assert(argc==3);directory=argv[1];mode=argv[2];boot_check_entry(NULL,NULL);return 1;}
'''
def main():
 here=Path(__file__).resolve().parent
 with tempfile.TemporaryDirectory() as d:
  root=Path(d);h=root/'harness.c';h.write_text(HARNESS);obj=root/'probe.o';exe=root/'probe'
  subprocess.run(['cc','-std=c11','-O1','-fno-builtin','-D_start=boot_check_entry','-D_init=boot_check_init','-D_fini=boot_check_fini','-c',str(here/'boot_check.c'),'-o',str(obj)],check=True)
  subprocess.run(['cc','-std=c11','-O2',str(obj),str(h),'-o',str(exe)],check=True)
  for mode in ['normal','denied','notify-failed']:
   target=root/mode;target.mkdir()
   subprocess.run([str(exe),str(target),mode],check=True,timeout=5)
   p=target/'probe.log'
   if mode=='denied':assert not p.exists()
   else:
    text=p.read_text();assert 'ENTRY_REACHED' in text and 'BOOT_CHECK_WAITING' in text
    assert ('notification request failed' if mode=='notify-failed' else 'returned success') in text
   print('PASS: native Boot Check entry',mode)
 print('3 host cases pass; native PS5 boot remains untested.')
if __name__=='__main__':main()
