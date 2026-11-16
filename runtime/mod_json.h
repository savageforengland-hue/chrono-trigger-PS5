/* SPDX-License-Identifier: GPL-3.0-or-later
 * Shared bounded JSON reader. No I/O; strict syntax, depth/token/byte limits. */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MJ_PATH 192
#define TOKENS 8192
#define BYTES (512u*1024u)
typedef struct MjToken {size_t start,end;int type;} MjToken;
typedef struct MjReader {const char*s;size_t n,at;MjToken*t;int count;} MjReader;
static inline void mj_white(MjReader*r){while(r->at<r->n&&strchr(" \r\n\t",r->s[r->at]))r->at++;}
static inline int mj_value(MjReader*r,int depth){
    mj_white(r);if(depth>16||r->at>=r->n||r->count==TOKENS)return -1;
    int id=r->count++;MjToken*t=r->t+id;t->start=r->at;char c=r->s[r->at++];t->type=c;
    if(c=='{'||c=='['){
        mj_white(r);char end=c=='{'?'}':']';
        if(r->at<r->n&&r->s[r->at]==end){r->at++;t->end=r->at;return id;}
        for(;;){
            if(c=='{'){
                mj_white(r);if(r->at>=r->n||r->s[r->at]!='"'||mj_value(r,depth+1)<0)return -1;
                mj_white(r);if(r->at>=r->n||r->s[r->at++]!=':')return -1;
            }
            if(mj_value(r,depth+1)<0)return -1;mj_white(r);if(r->at>=r->n)return -1;
            char mj_next=r->s[r->at++];if(mj_next==end)break;if(mj_next!=',')return -1;
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
static inline int mj_next(const MjReader*r,int i){size_t end=r->t[i].end;i++;while(i<r->count&&r->t[i].start<end)i++;return i;}
static inline int mj_equal(const MjReader*r,int i,const char*s){const MjToken*t=r->t+i;return t->type=='"'&&t->end-t->start==strlen(s)+2&&!memcmp(r->s+t->start+1,s,strlen(s));}
/* -1 missing; -2 duplicate key. */
static inline int mj_field(const MjReader*r,int object,const char*name){
    if(r->t[object].type!='{')return -2;int found=-1;
    for(int k=object+1;k<r->count&&r->t[k].start<r->t[object].end;){int v=k+1;if(mj_equal(r,k,name)){if(found!=-1)return -2;found=v;}k=mj_next(r,v);}return found;
}
static inline int mj_integer(const MjReader*r,int i,int lo,int hi,int*out){
    if(i<0||r->t[i].type!='0')return 0;const MjToken*t=r->t+i;unsigned n=0;
    for(size_t k=t->start;k<t->end;k++){unsigned char c=(unsigned char)r->s[k];if(c<'0'||c>'9'||n>100000u)return 0;n=n*10+c-'0';}
    if(n<(unsigned)lo||n>(unsigned)hi)return 0;*out=(int)n;return 1;
}
static inline int mj_boolean(const MjReader*r,int i,int*out){if(i==-1)return 1;if(i<0)return 0;if(r->t[i].type=='t'){*out=1;return 1;}if(r->t[i].type=='f'){*out=0;return 1;}return 0;}
static inline int mj_path(const MjReader*r,int i,char out[MJ_PATH]){
    if(i<0||r->t[i].type!='"')return 0;size_t n=r->t[i].end-r->t[i].start-2;
    if(n<5||n>=MJ_PATH)return 0;memcpy(out,r->s+r->t[i].start+1,n);out[n]=0;
    if(out[0]=='/'||strstr(out,"..")||strstr(out,"//")||strstr(out,"/./")||!strncmp(out,"./",2)||strcmp(out+n-4,".png"))return 0;
    for(size_t k=0;k<n;k++){unsigned char c=(unsigned char)out[k];if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||strchr("_-./",c)))return 0;}return 1;
}

static inline int mj_open(MjReader*r,const char*json,size_t length){
 memset(r,0,sizeof *r);if(!json||!length||length>BYTES||memchr(json,0,length))return 0;
 r->s=json;r->n=length;r->t=calloc(TOKENS,sizeof *r->t);if(!r->t)return 0;
 int root=mj_value(r,0);mj_white(r);if(root!=0||r->at!=length||r->t[0].type!='{'){free(r->t);r->t=NULL;return 0;}return 1;
}
static inline void mj_close(MjReader*r){free(r->t);r->t=NULL;}
static inline int mj_signed(const MjReader*r,int i,int lo,int hi,int*out){
 if(i<0||r->t[i].type!='0')return 0;size_t at=r->t[i].start,end=r->t[i].end;int sign=1;if(r->s[at]=='-'){sign=-1;at++;}int n=0;
 for(;at<end;at++){unsigned char c=(unsigned char)r->s[at];if(c<'0'||c>'9'||n>1000000)return 0;n=n*10+c-'0';}n*=sign;if(n<lo||n>hi)return 0;*out=n;return 1;
}
static inline int mj_string(const MjReader*r,int i,char*out,size_t capacity){
 if(i<0||r->t[i].type!='"'||!capacity)return 0;size_t at=r->t[i].start+1,end=r->t[i].end-1,n=0;
 while(at<end){unsigned c=(unsigned char)r->s[at++];if(c=='\\'){
  c=(unsigned char)r->s[at++];if(c=='u'){unsigned value=0;for(int k=0;k<4;k++){unsigned char h=(unsigned char)r->s[at++];value=value*16+(h<='9'?(unsigned)h-'0':(unsigned)tolower(h)-'a'+10);}c=value;}
  else if(c!='"'&&c!='\\'&&c!='/')return 0;
 }if(c<32||c>126||n+1>=capacity)return 0;out[n++]=(char)c;}out[n]=0;return 1;
}
static inline int mj_array_int(const MjReader*r,int token,int*out,int count,int lo,int hi){
 if(token<0||r->t[token].type!='[')return 0;int at=token+1;
 for(int i=0;i<count;i++){if(at>=r->count||r->t[at].start>=r->t[token].end||!mj_signed(r,at,lo,hi,out+i))return 0;at=mj_next(r,at);}return at>=r->count||r->t[at].start>=r->t[token].end;
}
static inline int mj_colour(const MjReader*r,int token,uint32_t*out){
 char s[12];if(!mj_string(r,token,s,sizeof s)||s[0]!='#')return 0;size_t n=strlen(s);if(n!=7&&n!=9)return 0;unsigned c[4]={0,0,0,255};
 for(size_t k=1;k<n;k++){int v=isdigit((unsigned char)s[k])?s[k]-'0':s[k]>='a'&&s[k]<='f'?s[k]-'a'+10:s[k]>='A'&&s[k]<='F'?s[k]-'A'+10:-1;if(v<0)return 0;c[(k-1)/2]=(k&1)?(unsigned)v*16:c[(k-1)/2]+(unsigned)v;}
 *out=c[0]|c[1]<<8|c[2]<<16|c[3]<<24;return 1;
}
