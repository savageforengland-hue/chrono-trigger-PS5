/* SPDX-License-Identifier: GPL-3.0-or-later
 * Original small PS5 prompt artwork. No pixels copied from game assets. */
#include "game_prompts.h"
#include <string.h>

static const unsigned char letters[36][5] = {
 {2,5,7,5,5},{6,5,6,5,6},{3,4,4,4,3},{6,5,5,5,6},{7,4,6,4,7},{7,4,6,4,4},
 {3,4,5,5,3},{5,5,7,5,5},{7,2,2,2,7},{1,1,1,5,2},{5,5,6,5,5},{4,4,4,4,7},
 {5,7,7,5,5},{5,7,7,7,5},{2,5,5,5,2},{6,5,6,4,4},{2,5,5,3,1},{6,5,6,5,5},
 {3,4,2,1,6},{7,2,2,2,2},{5,5,5,5,7},{5,5,5,5,2},{5,5,7,7,5},{5,5,2,5,5},
 {5,5,2,2,2},{7,1,2,4,7},
 {7,5,5,5,7},{2,6,2,2,7},{6,1,2,4,7},{6,1,2,1,6},{5,5,7,1,1},
 {7,4,6,1,6},{3,4,6,5,2},{7,1,2,2,2},{2,5,2,5,2},{2,5,3,1,6}
};
static void tiny(const char *text,unsigned char *out,int w,int h,int scale) {
 size_t len=strlen(text);int x0=(w-((int)len*4-1)*scale)/2,y0=(h-5*scale)/2;
 for(size_t i=0;i<len;i++){
  unsigned c=(unsigned char)text[i];
  if(c>='a'&&c<='z')c-=32;
  int g=c>='A'&&c<='Z'?(int)c-'A':c>='0'&&c<='9'?(int)c-'0'+26:-1;
  if(g<0)continue;
  for(int y=0;y<5;y++)for(int x=0;x<3;x++)if(letters[g][y]&(4>>x))
   for(int dy=0;dy<scale;dy++)for(int dx=0;dx<scale;dx++){
    int px=x0+((int)i*4+x)*scale+dx,py=y0+y*scale+dy;
    if(px>=0&&px<w&&py>=0&&py<h)out[py*w+px]=1;
   }
 }
}
void egg_ps5_game_prompt_mask(const char *label,unsigned char *out,int w,int h,int compact) {
 if(!out||w<=0||h<=0)return;
 memset(out,0,(size_t)w*h);
 if(!label)return;
 if(compact&&w<=5&&h>=5){
  const unsigned char *small=NULL;
  static const unsigned char circle[]={14,17,17,17,14},cross[]={17,10,4,10,17},square[]={31,17,17,17,31},triangle[]={4,10,10,17,31};
  if(!strcmp(label,"\xE2\x97\x8B"))small=circle;
  else if(!strcmp(label,"\xC3\x97"))small=cross;
  else if(!strcmp(label,"\xE2\x96\xA1"))small=square;
  else if(!strcmp(label,"\xE2\x96\xB3"))small=triangle;
  if(small){int x0=(w-5)/2,y0=(h-5)/2;for(int y=0;y<5;y++)for(int x=0;x<5;x++)if((small[y]&(16>>x))&&x+x0>=0)out[(y+y0)*w+x+x0]=1;return;}
 }
 unsigned char glyph[7]={0};int symbol=1;
 if(!strcmp(label,"\xE2\x97\x8B")){unsigned char a[]={28,34,65,65,65,34,28};memcpy(glyph,a,7);}
 else if(!strcmp(label,"\xC3\x97")){unsigned char a[]={65,34,20,8,20,34,65};memcpy(glyph,a,7);}
 else if(!strcmp(label,"\xE2\x96\xA1")){unsigned char a[]={127,65,65,65,65,65,127};memcpy(glyph,a,7);}
 else if(!strcmp(label,"\xE2\x96\xB3")){unsigned char a[]={8,20,20,34,34,65,127};memcpy(glyph,a,7);}
 else if(compact&&!strcmp(label,"Options")){unsigned char a[]={0,127,0,127,0,127,0};memcpy(glyph,a,7);}
 else if(compact&&!strcmp(label,"Create")){unsigned char a[]={34,20,0,62,34,34,62};memcpy(glyph,a,7);}
 else if(!strcmp(label,"\xE2\x86\x91")){unsigned char a[]={8,28,42,8,8,8,0};memcpy(glyph,a,7);}
 else if(!strcmp(label,"\xE2\x86\x93")){unsigned char a[]={0,8,8,8,42,28,8};memcpy(glyph,a,7);}
 else if(!strcmp(label,"\xE2\x86\x90")){unsigned char a[]={0,8,16,63,16,8,0};memcpy(glyph,a,7);}
 else if(!strcmp(label,"\xE2\x86\x92")){unsigned char a[]={0,8,4,126,4,8,0};memcpy(glyph,a,7);}
 else if(!strcmp(label,"-")){unsigned char a[]={0,0,0,28,0,0,0};memcpy(glyph,a,7);}
 else symbol=0;
 if(!symbol){int scale=!compact&&w>=16&&h>=12&&strlen(label)<=2?2:1;tiny(label,out,w,h,scale);return;}
 int scale=!compact&&w>=16&&h>=16?2:1,x0=(w-7*scale)/2,y0=(h-7*scale)/2;
 for(int y=0;y<7;y++)for(int x=0;x<7;x++)if(glyph[y]&(64>>x))
  for(int dy=0;dy<scale;dy++)for(int dx=0;dx<scale;dx++){
   int px=x0+x*scale+dx,py=y0+y*scale+dy;
   if(px>=0&&px<w&&py>=0&&py<h)out[py*w+px]=1;
  }
}
