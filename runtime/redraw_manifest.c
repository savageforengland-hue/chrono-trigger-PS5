/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "redraw_manifest.h"
#include "mod_json.h"
int redraw_manifest_parse(const char*json,size_t length,RedrawManifest*out,char*error,size_t capacity){
    if(!out)return 0;memset(out,0,sizeof *out);const char*reason="Invalid JSON manifest";
    if(!json||!length||length>BYTES||memchr(json,0,length))goto bad;
    MjToken*t=calloc(TOKENS,sizeof *t);RedrawManifest*m=calloc(1,sizeof *m);
    if(!t||!m){free(t);free(m);reason="Manifest allocation failed";goto bad;}
    MjReader r={json,length,0,t,0};int root=mj_value(&r,0);mj_white(&r);int schema;
    if(root!=0||r.at!=length||t[0].type!='{')goto done;
    reason="Unsupported manifest schema, character or scale";
    if(!mj_integer(&r,mj_field(&r,0,"schema"),2,2,&schema)||!mj_integer(&r,mj_field(&r,0,"character"),0,0,&m->character)||!mj_integer(&r,mj_field(&r,0,"scale"),4,4,&m->scale))goto done;
    m->enabled=1;reason="Invalid enabled flag";if(!mj_boolean(&r,mj_field(&r,0,"enabled"),&m->enabled))goto done;
    int frames=mj_field(&r,0,"frames");reason="Missing frames array";if(frames<0||t[frames].type!='[')goto done;
    for(int k=frames+1;k<r.count&&t[k].start<t[frames].end;k=mj_next(&r,k)){
        reason="Invalid frame entry";if(t[k].type!='{'||m->count==REDRAW_MANIFEST_FRAMES)goto done;
        RedrawFrameSpec*f=m->frames+m->count;f->enabled=1;
        if(!mj_integer(&r,mj_field(&r,k,"cell"),0,65535,&f->cell)||!mj_path(&r,mj_field(&r,k,"file"),f->file)||!mj_boolean(&r,mj_field(&r,k,"enabled"),&f->enabled))goto done;
        reason="Duplicate frame cell or PNG mj_path";for(int j=0;j<m->count;j++)if(m->frames[j].cell==f->cell||!strcmp(m->frames[j].file,f->file))goto done;
        m->count++;
    }
    reason="Empty frames array";if(!m->count)goto done;
    *out=*m;free(t);free(m);if(error&&capacity)error[0]=0;return 1;
done:free(t);free(m);
bad:if(error&&capacity)snprintf(error,capacity,"%s",reason);return 0;
}
