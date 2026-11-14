#!/usr/bin/env python3
"""Host integration test of PS5 logging paths and actual fatal-signal delivery.

The shim models kernel file APIs; this does not certify PS5 signal ABI or mounts.
"""
import os
from pathlib import Path
import signal
import subprocess
import tempfile

HARNESS = r'''
#include "diagnostics.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
static const char *root;
int sceKernelMkdir(const char *p, unsigned m) { (void)p; (void)m; return 0; }
int sceKernelOpen(const char *p, int f, unsigned m) {
 const char *mode=getenv("DIAG_TEST_MODE");
 if (!strcmp(mode,"none") || (!strcmp(mode,"fallback") && !strncmp(p,"/data/",6))) return -EACCES;
 char out[1024];snprintf(out,sizeof out,"%s/%s",root,!strncmp(p,"/data/",6)?"primary.log":!strncmp(p,"/download0/",11)?"fallback.log":"temporary.log");
 int fd=open(out,f,m);return fd<0?-errno:fd;
}
ssize_t sceKernelWrite(int fd,const void *p,size_t n) {
 if(getenv("DIAG_TEST_PARTIAL") && n>7)n=7;
 return write(fd,p,n);
}
int sceKernelFsync(int fd) {return fsync(fd);}
int sceKernelClose(int fd) {return close(fd);}
int dup2(int oldfd,int newfd) {(void)oldfd;(void)newfd;errno=EPERM;return -1;}
int sceKernelDebugOutText(int channel,const char *text) {
 (void)channel;char name[1024];snprintf(name,sizeof name,"%s/klog.txt",root);
 FILE *f=fopen(name,"a");if(!f)return -1;fputs(text,f);fclose(f);return 0;
}
int sceKernelSendNotificationRequest(uint32_t d,void *p,size_t n,uint32_t b) {
 (void)d;(void)n;(void)b;
 char name[1024];snprintf(name,sizeof name,"%s/notification.txt",root);
 FILE *f=fopen(name,"a");if(!f)return -1;
 fprintf(f,"%s\n",(char*)p+45);fclose(f);return 0;
}
int main(int argc,char **argv) {
 struct rlimit limit={0,0};setrlimit(RLIMIT_CORE,&limit);
 root=argv[1];
 egg_ps5_diag_early();
 egg_ps5_diag_stage(10,"before _init_env");
 egg_ps5_diag_ready();
 egg_ps5_diag_stage(40,"SDL_Init begin");
 fprintf(stderr,"stderr reaches startup log\n");
 char long_message[1001];memset(long_message,'Q',1000);long_message[1000]=0;
 egg_ps5_diag_message(long_message);
 if(argc>2 && !strcmp(argv[2],"segv"))raise(SIGSEGV);
 if(argc>2 && !strcmp(argv[2],"abort"))abort();
 if(argc>2 && !strcmp(argv[2],"long-failure")) {
  char detail[8192];memset(detail,'X',sizeof detail-1);detail[sizeof detail-1]=0;
  egg_ps5_diag_failure("SDL_Init",detail);
 } else egg_ps5_diag_failure("SDL_Init","simulated video failure");
 egg_ps5_diag_exit(1);return 0;
}
'''


def main():
    here = Path(__file__).resolve().parent
    with tempfile.TemporaryDirectory() as temporary:
        temp = Path(temporary)
        harness = temp/'harness.c';harness.write_text(HARNESS)
        executable=temp/'test'
        subprocess.run(['cc','-D_GNU_SOURCE','-std=c11','-O2','-fno-builtin','-I',str(here),
                        str(here/'diagnostics.c'),str(harness),'-o',str(executable)],check=True)
        for mode in ('primary','fallback','none'):
            for failure in ('normal','segv','abort'):
                directory=temp/(mode+'-'+failure);directory.mkdir()
                env=dict(os.environ,DIAG_TEST_MODE=mode)
                run=subprocess.run([str(executable),str(directory),failure],env=env,capture_output=True,text=True,timeout=10)
                expected={'normal':0,'segv':-signal.SIGSEGV,'abort':-signal.SIGABRT}[failure]
                assert run.returncode==expected,(mode,failure,run.returncode,run.stderr)
                if mode=='none':
                    assert not list(directory.glob('*.log'))
                    text=run.stderr
                    assert 'file logging unavailable' in (directory/'notification.txt').read_text()
                else:
                    path=directory/('primary.log' if mode=='primary' else 'fallback.log')
                    text=path.read_text()
                    notifications=directory/'notification.txt'
                    assert not notifications.exists() or 'startup log (app path)' not in notifications.read_text()
                assert 'CTPS5 0.1.0-rc1 entry reached' in text
                assert 'stage=10 before _init_env' in text
                assert 'stage=40 SDL_Init begin' in text
                kernel=(directory/'klog.txt').read_text()
                assert '[CTPS5 RC1] stage=1 ' not in kernel
                assert '[CTPS5 RC1] stage=20' in kernel
                assert '[CTPS5 RC1] stage=40 SDL_Init begin' in kernel
                assert 'stderr reaches startup log' in text
                if failure=='normal':
                    assert 'STARTUP FAILURE SDL_Init: simulated video failure' in text
                    assert 'main returned status=1' in text
                    # Includes no-file mode: the actual cause must be visible
                    # even if the game log cannot be retrieved through FTP.
                    assert 'CTPS5 RC1 SDL_Init: simulated video failure' in (directory/'notification.txt').read_text()
                else:
                    assert 'FATAL signal=' in text and 'last_stage=40' in text
                    assert 'native_pc=0x' in text and 'fault_address=0x' in text
                if mode=='fallback':assert 'result=0xfffffff3' in text
                print('pass:',mode,failure)
        # Force partial kernel writes; preserve each diagnostic line completely.
        directory=temp/'partial';directory.mkdir()
        env=dict(os.environ,DIAG_TEST_MODE='primary',DIAG_TEST_PARTIAL='1')
        run=subprocess.run([str(executable),str(directory),'normal'],env=env,capture_output=True,text=True,timeout=10)
        assert run.returncode==0
        assert 'STARTUP FAILURE SDL_Init: simulated video failure' in (directory/'primary.log').read_text()
        assert 'Q'*1000 in (directory/'primary.log').read_text()
        print('pass: partial kernel writes; native dup2 refusal modeled throughout')
        directory=temp/'long-failure';directory.mkdir()
        env=dict(os.environ,DIAG_TEST_MODE='none')
        run=subprocess.run([str(executable),str(directory),'long-failure'],env=env,capture_output=True,text=True,timeout=10)
        assert run.returncode==0
        notification=(directory/'notification.txt').read_text().splitlines()[-1]
        assert notification.startswith('CTPS5 RC1 SDL_Init: '+'X'*100)
        assert len(notification)==2046
        print('pass: bounded notification with an 8191-byte error and no log file')
    print('11 host integration cases passed. PS5 mount/signal behavior remains untested.')


if __name__=='__main__':main()
