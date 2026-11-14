/* SPDX-License-Identifier: GPL-3.0-or-later
 * Native home-screen boot check. No SDL, ROM, allocator, C runtime or payload
 * loader is needed. Only public kernel and SystemService imports are used.
 * A successful check proves entry only; it is not a game/graphics test.
 */
#include <stddef.h>
#include <stdint.h>
#include <fcntl.h>
extern int sceKernelOpen(const char *,int,unsigned);
extern long sceKernelWrite(int,const void *,size_t);
extern int sceKernelFsync(int);
extern int sceKernelSendNotificationRequest(unsigned,void *,size_t,unsigned);
extern int sceKernelUsleep(unsigned);
extern int sceSystemServiceHideSplashScreen(void);

void _start(void *,void *);
/* Exercise the same nonempty data-before-GOT layout that R5 now validates. */
__attribute__((used)) static void (*const entry_reference)(void *,void *) = _start;
static volatile int probe_state=1;
static struct { unsigned char reserved[45];char message[3075]; } notification;
static void record(int fd,const char *message) {
    size_t length=0;while(message[length])++length;
    if(fd>=0) {
        while(length) {
            long done=sceKernelWrite(fd,message,length);
            if(done<=0 || (size_t)done>length)break;
            message+=done;length-=(size_t)done;
        }
        (void)sceKernelFsync(fd);
    }
}
void _init(void) {}
void _fini(void) {}
__attribute__((noreturn)) void _start(void *parameters,void *teardown) {
    (void)parameters;(void)teardown;
    int fd=sceKernelOpen("/data/eggoftime-boot-check.log",O_WRONLY|O_CREAT|O_APPEND,0666);
    record(fd,"\nEgg native Boot Check v1: ENTRY_REACHED\n");
    const char *message=fd>=0?
        "Egg Boot Check: native entry reached. Log: /data/eggoftime-boot-check.log":
        "Egg Boot Check: native entry reached; log file could not be opened.";
    size_t i=0;
    while(message[i] && i<sizeof notification.message-1){notification.message[i]=message[i];++i;}
    notification.message[i]=0;
    int result=sceKernelSendNotificationRequest(0,&notification,sizeof notification,0);
    record(fd,result<0?"notification request failed\n":"notification request returned success\n");
    (void)sceSystemServiceHideSplashScreen();
    record(fd,"BOOT_CHECK_WAITING: close this test from the PS button menu\n");
    probe_state=2;
    for(;;)(void)sceKernelUsleep(100000);
}
