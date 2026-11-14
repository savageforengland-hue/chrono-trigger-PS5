/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "config_file.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
int egg_ps5_ini_set(const char *path,const char *key,const char *value){
    char temp[1024],line[1024];
    if(!path||!key||!value||strchr(value,'\n')||strchr(value,'\r')||strchr(key,'=')||!key[0])return 0;
    if(snprintf(temp,sizeof temp,"%s.tmp",path)>=(int)sizeof temp)return 0;
    FILE*in=fopen(path,"r"),*out=fopen(temp,"w");if(!out){if(in)fclose(in);return 0;}
    int wrote=0,ok=1;size_t n=strlen(key);
    while(in&&fgets(line,sizeof line,in)){
        /* Reject oversized lines without touching the existing file. */
        if(!strchr(line,'\n')&&!feof(in)){ok=0;break;}
        char*p=line;while(*p==' '||*p=='\t')p++;
        char*q=p;if(!strncmp(p,key,n)){q=p+n;while(*q==' '||*q=='\t')q++;}
        if(q!=p&&*q=='='){
            if(!wrote)fprintf(out,"%s=%s\n",key,value);
            wrote=1;
        }else{
            fputs(line,out);
            if(!strchr(line,'\n'))fputc('\n',out);
        }
    }
    if(in){if(ferror(in))ok=0;if(fclose(in))ok=0;}
    if(!wrote)fprintf(out,"%s=%s\n",key,value);
    if(ferror(out))ok=0;
    if(fclose(out))ok=0;
    if(ok&&rename(temp,path)==0)return 1;
    unlink(temp);return 0;
}
