/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "runtime/redraw_manifest.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char*header="{\"schema\":2,\"character\":0,\"scale\":4,\"frames\":";
static void invalid(const char*s){RedrawManifest m;char e[160];memset(&m,0x5a,sizeof m);assert(!redraw_manifest_parse(s,strlen(s),&m,e,sizeof e)&&e[0]&&!m.count);}
int main(void){
    RedrawManifest*m=malloc(sizeof *m);assert(m);char error[160];
    const char*good="{\"name\":\"Crono \\\"test\\\"\",\"schema\":2,\"character\":0,\"scale\":4,\"enabled\":true,\"notes\":{\"ignored\":[null,false,1.2e+3]},\"frames\":[{\"cell\":0,\"file\":\"frames/Chara/Chara_0000/000.png\"},{\"cell\":56,\"file\":\"056.png\",\"enabled\":false}]}";
    assert(redraw_manifest_parse(good,strlen(good),m,error,sizeof error)&&m->count==2&&m->enabled&&m->frames[0].enabled&&!m->frames[1].enabled&&m->frames[1].cell==56);
    const char*bad[]={"{}","{\"schema\":2,}","[]","true","{\"schema\":2,\"character\":1,\"scale\":4,\"frames\":[]}","{\"schema\":2,\"schema\":2,\"character\":0,\"scale\":4,\"frames\":[]}"};
    for(unsigned i=0;i<sizeof bad/sizeof *bad;i++)invalid(bad[i]);
    const char*frames[]={"[]","[{\"cell\":-1,\"file\":\"000.png\"}]","[{\"cell\":0,\"file\":\"../000.png\"}]","[{\"cell\":0,\"file\":\"/000.png\"}]","[{\"cell\":0,\"file\":\"frames\\\\000.png\"}]","[{\"cell\":0,\"file\":\"frames//000.png\"}]","[{\"cell\":0,\"file\":\"000.jpg\"}]","[{\"cell\":0,\"file\":\"000.png\",\"enabled\":1}]","[{\"cell\":0,\"file\":\"000.png\"},{\"cell\":0,\"file\":\"001.png\"}]","[{\"cell\":0,\"file\":\"000.png\"},{\"cell\":1,\"file\":\"000.png\"}]","[{\"cell\":01,\"file\":\"000.png\"}]","[{\"cell\":1.5,\"file\":\"000.png\"}]","[{\"cell\":0,\"file\":\"000.png\",}]"};
    char text[2048];for(unsigned i=0;i<sizeof frames/sizeof *frames;i++){snprintf(text,sizeof text,"%s%s}",header,frames[i]);invalid(text);}
    for(size_t i=1;i<strlen(good);i++){memcpy(text,good,i);text[i]=0;invalid(text);}
    snprintf(text,sizeof text,"%s junk",good);invalid(text);free(m);
    puts("PASS: JSON manifest, metadata skipping, per-frame flags, truncated/malformed input, duplicate cells/paths and unsafe paths rejected without partial output");return 0;
}
