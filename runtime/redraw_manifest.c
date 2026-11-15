/* SPDX-License-Identifier: GPL-3.0-or-later
 * Bounded JSON reader for authored-pose manifests. Unknown metadata is parsed
 * and skipped; frame identity/path/scale are validated before any art loads. */
#include "redraw_manifest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#define TOKENS 8192
#define BYTES (512u*1024u)
typedef struct Token {size_t start,end;int type;} Token;
typedef struct Reader {const char*s;size_t n,at;Token*t;int count;} Reader;
static void white(Reader*r){while(r->at<r->n&&strchr(" \r\n\t",r->s[r->at]))r->at++;}
static int value(Reader*r,int depth){
    white(r);if(depth>16||r->at>=r->n||r->count==TOKENS)return -1;
    int id=r->count++;Token*t=r->t+id;t->start=r->at;char c=r->s[r->at++];t->type=c;
    if(c=='{'||c=='['){
        white(r);char end=c=='{'?'}':']';
        if(r->at<r->n&&r->s[r->at]==end){r->at++;t->end=r->at;return id;}
        for(;;){
            if(c=='{'){
                white(r);if(r->at>=r->n||r->s[r->at]!='"'||value(r,depth+1)<0)return -1;
                white(r);if(r->at>=r->n||r->s[r->at++]!=':')return -1;
            }
            if(value(r,depth+1)<0)return -1;white(r);if(r->at>=r->n)return -1;
            char next=r->s[r->at++];if(next==end)break;if(next!=',')return -1;
        }
    }else if(c=='"'){
        int closed=0;while(r->at<r->n){unsigned char x=(unsigned char)r->s[r->at++];if(x=='"'){closed=1;break;}if(x<32)return -1;
            if(x=='\\'){if(r->at>=r->n)return -1;char e=r->s[r->at++];if(e=='u'){for(int i=0;i<4;i++)if(r->at>=r->n||!isxdigit((unsigned char)r->s[r->at++]))return -1;}else if(!strchr("\"\\/bfnrt",e))return -1;}
        }if(!closed)return -1;
    }else if(c=='t'||c=='f'||c=='n'){
        const char*word=c=='t'?"true":c=='f'?"false":"null";size_t len=strlen(word);
        if(r->n-t->start<len||memcmp(r->s+t->start,word,len))return -1;r->at=t->start+len;
    }else{
        r->at=t->start;if(r->s[r->at]=='-')r->at++;
        if(r->at>=r->n||!isdigit((unsigned char)r->s[r->at]))return -1;
        if(r->s[r->at]=='0')r->at++;else while(r->at<r->n&&isdigit((unsigned char)r->s[r->at]))r->at++;
        if(r->at<r->n&&r->s[r->at]=='.'){r->at++;size_t start=r->at;while(r->at<r->n&&isdigit((unsigned char)r->s[r->at]))r->at++;if(start==r->at)return -1;}
        if(r->at<r->n&&(r->s[r->at]=='e'||r->s[r->at]=='E')){r->at++;if(r->at<r->n&&(r->s[r->at]=='+'||r->s[r->at]=='-'))r->at++;size_t start=r->at;while(r->at<r->n&&isdigit((unsigned char)r->s[r->at]))r->at++;if(start==r->at)return -1;}
        t->type='0';
    }
    t->end=r->at;return id;
}
static int next(const Reader*r,int i){size_t end=r->t[i].end;i++;while(i<r->count&&r->t[i].start<end)i++;return i;}
static int equal(const Reader*r,int i,const char*s){const Token*t=r->t+i;return t->type=='"'&&t->end-t->start==strlen(s)+2&&!memcmp(r->s+t->start+1,s,strlen(s));}
/* -1 missing; -2 duplicate key. */
static int field(const Reader*r,int object,const char*name){
    if(r->t[object].type!='{')return -2;int found=-1;
    for(int k=object+1;k<r->count&&r->t[k].start<r->t[object].end;){int v=k+1;if(equal(r,k,name)){if(found!=-1)return -2;found=v;}k=next(r,v);}return found;
}
static int integer(const Reader*r,int i,int lo,int hi,int*out){
    if(i<0||r->t[i].type!='0')return 0;const Token*t=r->t+i;unsigned n=0;
    for(size_t k=t->start;k<t->end;k++){unsigned char c=(unsigned char)r->s[k];if(c<'0'||c>'9'||n>100000u)return 0;n=n*10+c-'0';}
    if(n<(unsigned)lo||n>(unsigned)hi)return 0;*out=(int)n;return 1;
}
static int boolean(const Reader*r,int i,int*out){if(i==-1)return 1;if(i<0)return 0;if(r->t[i].type=='t'){*out=1;return 1;}if(r->t[i].type=='f'){*out=0;return 1;}return 0;}
static int path(const Reader*r,int i,char out[REDRAW_MANIFEST_PATH]){
    if(i<0||r->t[i].type!='"')return 0;size_t n=r->t[i].end-r->t[i].start-2;
    if(n<5||n>=REDRAW_MANIFEST_PATH)return 0;memcpy(out,r->s+r->t[i].start+1,n);out[n]=0;
    if(out[0]=='/'||strstr(out,"..")||strstr(out,"//")||strstr(out,"/./")||!strncmp(out,"./",2)||strcmp(out+n-4,".png"))return 0;
    for(size_t k=0;k<n;k++){unsigned char c=(unsigned char)out[k];if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||strchr("_-./",c)))return 0;}return 1;
}
int redraw_manifest_parse(const char*json,size_t length,RedrawManifest*out,char*error,size_t capacity){
    if(!out)return 0;memset(out,0,sizeof *out);const char*reason="Invalid JSON manifest";
    if(!json||!length||length>BYTES||memchr(json,0,length))goto bad;
    Token*t=calloc(TOKENS,sizeof *t);RedrawManifest*m=calloc(1,sizeof *m);
    if(!t||!m){free(t);free(m);reason="Manifest allocation failed";goto bad;}
    Reader r={json,length,0,t,0};int root=value(&r,0);white(&r);int schema;
    if(root!=0||r.at!=length||t[0].type!='{')goto done;
    reason="Unsupported manifest schema, character or scale";
    if(!integer(&r,field(&r,0,"schema"),2,2,&schema)||!integer(&r,field(&r,0,"character"),0,0,&m->character)||!integer(&r,field(&r,0,"scale"),4,4,&m->scale))goto done;
    m->enabled=1;reason="Invalid enabled flag";if(!boolean(&r,field(&r,0,"enabled"),&m->enabled))goto done;
    int frames=field(&r,0,"frames");reason="Missing frames array";if(frames<0||t[frames].type!='[')goto done;
    for(int k=frames+1;k<r.count&&t[k].start<t[frames].end;k=next(&r,k)){
        reason="Invalid frame entry";if(t[k].type!='{'||m->count==REDRAW_MANIFEST_FRAMES)goto done;
        RedrawFrameSpec*f=m->frames+m->count;f->enabled=1;
        if(!integer(&r,field(&r,k,"cell"),0,65535,&f->cell)||!path(&r,field(&r,k,"file"),f->file)||!boolean(&r,field(&r,k,"enabled"),&f->enabled))goto done;
        reason="Duplicate frame cell or PNG path";for(int j=0;j<m->count;j++)if(m->frames[j].cell==f->cell||!strcmp(m->frames[j].file,f->file))goto done;
        m->count++;
    }
    reason="Empty frames array";if(!m->count)goto done;
    *out=*m;free(t);free(m);if(error&&capacity)error[0]=0;return 1;
done:free(t);free(m);
bad:if(error&&capacity)snprintf(error,capacity,"%s",reason);return 0;
}
