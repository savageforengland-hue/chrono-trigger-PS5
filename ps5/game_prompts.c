/* SPDX-License-Identifier: GPL-3.0-or-later
 * Replace game-owned button sprites and explicit button instructions using the
 * player's verified USA ROM. All changes are in-memory data, not game code.
 * NCER layouts locate the actual shared tiles, including composite prompts. */
#include "game_prompts.h"
#include "../runtime/controls.h"
#include "../runtime/gfx_rom.h"
#include "../runtime/rt.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

typedef struct Edit { uint32_t file_id; unsigned char *data; uint32_t size; int levels; } Edit;
static Edit *edits;
static int edit_count, sprite_count, text_count;
static void w32(unsigned char*p,uint32_t v){for(int k=0;k<4;k++)p[k]=(unsigned char)(v>>(8*k));}
static Edit *stage(const GfxFile *file,const GfxBlob *blob){
 if(!file||!blob->data||!blob->size)return NULL;
 Edit *list=realloc(edits,(size_t)(edit_count+1)*sizeof *list);if(!list)return NULL;edits=list;
 Edit *e=&edits[edit_count];e->data=malloc(blob->size);if(!e->data)return NULL;
 memcpy(e->data,blob->data,blob->size);e->file_id=file->id;e->size=blob->size;e->levels=blob->levels;edit_count++;return e;
}
static int ink(const GfxPalette *pal,int dim){
 int best=1,dist=INT_MAX,target=dim?145:255;
 for(int i=1;i<16;i++){int r=pal->c[i].r-target,g=pal->c[i].g-target,b=pal->c[i].b-target,d=r*r+g*g+b*b;if(d<dist)best=i,dist=d;}
 return best;
}
static int stamp(Edit *sheet,const GfxTiles *tiles,const GfxLayout *l,const GfxPalette *pal,int action,int dim,int field){
 int count=field?4:l->count,w=field?16:l->w,h=field==1?9:field==2?16:l->h,x0=field==1?4:field==2?5:0;
 if(tiles->bpp8||l->bpp8||count!=4||w>32||h>16)return 0;
 /* Reject unexpected layouts rather than touching another graphic. */
 for(int i=0;i<count;i++){
  const GfxPlace*p=&l->tiles[i];int x=p->x-x0,y=p->y;
  if(p->bpp8||p->row||x<0||x>=w||y<0||y>=16||p->off>tiles->size||tiles->size-p->off<32)return 0;
 }
 unsigned char mask[32*16];egg_ps5_game_prompt_mask(controls_pad_label(action),mask,w,h,field==1);
 int color=ink(pal,dim);
 for(int i=0;i<count;i++){
  const GfxPlace*p=&l->tiles[i];unsigned char *at=sheet->data+tiles->offset+p->off;
  for(int y=0;y<8;y++)for(int x=0;x<8;x++){
   int px=p->x-x0+x,py=p->y+y;
   int v=px>=0&&px<w&&py>=0&&py<h&&mask[py*w+px]?color:0;
   int tx=p->hflip?7-x:x,ty=p->vflip?7-y:y,k=ty*4+tx/2;
   at[k]=(unsigned char)(tx&1?(at[k]&15)|(v<<4):(at[k]&240)|v);
  }
 }
 sprite_count++;return 1;
}
static void sheet(const char *name,int family){
 char path[180];snprintf(path,sizeof path,"menu/obj/%s.NCGR",name);const GfxFile *file=gfx_find(path);
 snprintf(path,sizeof path,"menu/obj/%s.NCER",name);const GfxFile *cells=gfx_find(path);if(!file||!cells)return;
 GfxBlob b=gfx_open(file),cb=gfx_open(cells);GfxTiles t;GfxPalette pal;
 if(!gfx_read_tiles(&b,&t)){gfx_close(&b);gfx_close(&cb);return;}
 Edit *e=stage(file,&b);gfx_palette_for(file->path,0,&pal);
 if(e){
  int start=family==1?11:0,count=family==2?8:family==3?6:family==4?3:16;
  static const int actions[]={CTL_A,CTL_B,CTL_X,CTL_Y,CTL_L,CTL_R,CTL_START,CTL_SELECT};
  for(int i=0;i<count;i++){
   int c=start+i,action=actions[i/2];
   if(family==4)action=CTL_B;
   if(family==3)action=i<4?(i<2?CTL_L:CTL_R):CTL_SELECT;
   GfxLayout l;if(gfx_cell_layout(&cb,(uint32_t)c,&l)){stamp(e,&t,&l,&pal,action,family==4?i<2:i&1,family==2?1:family==4?2:0);free(l.tiles);}
  }
  if(family==3)for(int i=0;i<8;i++){
   GfxLayout l;if(gfx_cell_layout(&cb,(uint32_t)(54+i),&l)){stamp(e,&t,&l,&pal,actions[i/2],i&1,0);free(l.tiles);}
  }
 }
 gfx_close(&b);gfx_close(&cb);
}
/* Game messages use UTF-8-shaped glyph numbers, not Unicode. Keep the font
 * count, ASCII table and every existing control opcode unchanged. */
static int next(const unsigned char*d,size_t n,size_t *at,unsigned *value){
 if(*at>=n)return 0;
 unsigned a=d[(*at)++];
 if(a<128){*value=a;return 1;}int k=a<224?1:2;unsigned v=a&(k==1?31:15);
 for(int i=0;i<k;i++){if(*at>=n||(d[*at]&192)!=128)return 0;v=(v<<6)|(d[(*at)++]&63);}
 *value=v;return 1;
}
typedef struct Buf { unsigned char *data;size_t size,cap; } Buf;
static int push(Buf*b,const void*p,size_t n){
 if(n>SIZE_MAX-b->size)return 0;
 size_t need=b->size+n;
 if(need>b->cap){size_t cap=need>SIZE_MAX/2?need:need*2;unsigned char*q=realloc(b->data,cap);if(!q)return 0;b->data=q;b->cap=cap;}
 memcpy(b->data+b->size,p,n);b->size=need;return 1;
}
static int code(Buf*b,unsigned c){unsigned char v[3];size_t n;
 if(c<128){v[0]=(unsigned char)c;n=1;}else if(c<2048){v[0]=(unsigned char)(192|(c>>6));v[1]=(unsigned char)(128|(c&63));n=2;}
 else{v[0]=(unsigned char)(224|(c>>12));v[1]=(unsigned char)(128|((c>>6)&63));v[2]=(unsigned char)(128|(c&63));n=3;}return push(b,v,n);
}
static unsigned ascii(const GfxFont*f,unsigned c){return gfx_le16(f->data+12+c*2);}
static int match(const GfxFont*f,const unsigned*cp,size_t n,size_t i,const char*s){
 for(size_t k=0;s[k];k++)if(i+k>=n||cp[i+k]!=ascii(f,(unsigned char)s[k]))return 0;
 return 1;
}
static int contains(const GfxFont*f,const unsigned*cp,size_t n,const char*s){for(size_t i=0;i<n;i++)if(match(f,cp,n,i,s))return 1;
 return 0;}
static int word(const GfxFont*f,unsigned cp){
 for(unsigned c='A';c<='Z';c++)if(cp==ascii(f,c)||cp==ascii(f,c+32))return 1;
 for(unsigned c='0';c<='9';c++)if(cp==ascii(f,c))return 1;
 return 0;
}
static GfxBlob message_open(const GfxFile *file){
 if(!file||file->size<16||file->offset>rt_rom.size||file->size>rt_rom.size-file->offset)return (GfxBlob){0};
 const unsigned char *data=rt_rom.data+file->offset;
 if(memcmp(data,"\0\0\0\0TEXT",8))return (GfxBlob){0};
 return (GfxBlob){data,file->size,NULL,0};
}
static int text_header(const GfxBlob*b,size_t *count,unsigned *end,const GfxFont*f){
 if(!b->data||b->size<16||memcmp(b->data,"\0\0\0\0TEXT",8))return 0;
 *count=(size_t)b->data[8]*gfx_le16(b->data+9);*end=ascii(f,0);
 return *end<128&&*count<=(b->size-16)/4;
}
static int read_string(const GfxBlob*b,size_t index,unsigned end,unsigned **out,size_t *count){
 uint32_t at=gfx_le32(b->data+16+index*4);if(at>=b->size)return 0;size_t stop=at;
 while(stop<b->size&&b->data[stop]!=end)stop++;
 if(stop==b->size)return 0;
 unsigned *cp=malloc((stop-at+1)*sizeof *cp);if(!cp)return 0;size_t n=0,pos=at;
 while(pos<stop){if(!next(b->data,stop,&pos,&cp[n++])){free(cp);return 0;}}
 *out=cp;*count=n;return 1;
}
static int rewrite(Buf*out,const GfxFont*f,const unsigned*cp,size_t n,const unsigned slots[6]){
 static const char keys[]="ABXYLR";int changed=0;
 int button_context=contains(f,cp,n,"Button")||contains(f,cp,n,"button");
 int crane=contains(f,cp,n,"Crane control codes:")||contains(f,cp,n,"Code de la grue");
 int list=contains(f,cp,n,"letters A, B, X")||crane;
 for(size_t i=0;i<n;){
  int boundary=!i||!word(f,cp[i-1]);const char*replacement=NULL;size_t consumed=0;
  if(boundary&&match(f,cp,n,i,"START")&&(i+5==n||!word(f,cp[i+5])))replacement=controls_pad_label(CTL_START),consumed=5;
  else if(boundary&&match(f,cp,n,i,"SELECT")&&(i+6==n||!word(f,cp[i+6])))replacement=controls_pad_label(CTL_SELECT),consumed=6;
  if(!replacement&&list&&match(f,cp,n,i,"letters")){replacement="symbols";consumed=7;}
  if(replacement){
   /* Options/Create default names; arbitrary remapped inputs use the same
    * authored compact glyph as the six button references where applicable. */
   if(!strcmp(replacement,"Options")||!strcmp(replacement,"Create")||!strcmp(replacement,"symbols")){
    for(size_t k=0;replacement[k];k++)if(!code(out,ascii(f,(unsigned char)replacement[k])))return -1;
   }else{
    /* Keep a readable PS5 name without inserting Unicode into game encoding. */
    const char *name=!strcmp(replacement,"\xE2\x97\x8B")?"Circle":!strcmp(replacement,"\xC3\x97")?"Cross":!strcmp(replacement,"\xE2\x96\xA1")?"Square":!strcmp(replacement,"\xE2\x96\xB3")?"Triangle":!strcmp(replacement,"\xE2\x86\x91")?"Up":!strcmp(replacement,"\xE2\x86\x93")?"Down":!strcmp(replacement,"\xE2\x86\x90")?"Left":!strcmp(replacement,"\xE2\x86\x92")?"Right":replacement;
    for(size_t k=0;name[k]&&(unsigned char)name[k]<128;k++)if(!code(out,ascii(f,(unsigned char)name[k])))return -1;
   }
   i+=consumed;changed++;continue;
  }
  int action=-1;
  if(boundary&&(i+1==n||!word(f,cp[i+1])))for(int k=0;k<6;k++)if(cp[i]==ascii(f,(unsigned char)keys[k])){
   int french=i>=7&&(match(f,cp,n,i-7,"bouton ")||match(f,cp,n,i-7,"Bouton "));
   int quoted=button_context&&i>0&&cp[i-1]==ascii(f,'"')&&match(f,cp,n,i+1,"\" icons");
   int chain=button_context&&(match(f,cp,n,i+1,",")||match(f,cp,n,i+1," and ")||match(f,cp,n,i+1," or "));
   if(match(f,cp,n,i+1," Button")||match(f,cp,n,i+1," button")||french||chain||list||quoted)action=k;
  }
  if(!code(out,action<0?cp[i]:slots[action]))return -1;
  changed+=action>=0;i++;
 }
 return changed;
}
static int in_folder(const GfxFile*file,const char*folder){size_t n=strlen(folder);return !strncmp(file->path,folder,n)&&gfx_ends_with(file->path,".msg");}
static void font_messages(const char *folder){
 char path[120];snprintf(path,sizeof path,"%smsgcmn.fnt",folder);const GfxFile*file=gfx_find(path);if(!file)return;
 GfxBlob fb=gfx_open(file);GfxFont font;
 if(!gfx_read_font(&fb,&font)||font.bpp!=2||font.height<8||font.height>16){gfx_close(&fb);return;}
 unsigned char *used=calloc((size_t)font.count,1);if(!used){gfx_close(&fb);return;}
 for(int c=0;c<128;c++){unsigned g=ascii(&font,(unsigned)c);if(g<(unsigned)font.count)used[g]=1;}
 int files=gfx_list_files(),valid=1; /* Rebuild the lookup after any earlier graphics edits (still staged). */
 file=gfx_find(path);
 for(int i=0;i<files&&valid;i++)if(in_folder(gfx_file(i),folder)){
  GfxBlob b=message_open(gfx_file(i));size_t count;unsigned end;
  if(!text_header(&b,&count,&end,&font)){valid=0;gfx_close(&b);continue;}
  for(size_t j=0;j<count&&valid;j++){
   unsigned*cp;size_t n;if(!read_string(&b,j,end,&cp,&n)){valid=0;break;}
   for(size_t k=0;k<n;k++)if(cp[k]<(unsigned)font.count)used[cp[k]]=1;
   free(cp);
  }gfx_close(&b);
 }
 unsigned slots[6];int found=0;
 if(valid)for(unsigned i=0;i<(unsigned)font.count&&found<6;i++)if(!used[i])slots[found++]=i;
 free(used);if(found<6){rt_log("PS5 game prompts: no safe spare glyphs in %s; text unchanged",folder);gfx_close(&fb);return;}
 Edit *fe=stage(file,&fb);if(!fe){gfx_close(&fb);return;}
 size_t extra=6u*(2u+(unsigned)font.height*2u);unsigned char *grown=realloc(fe->data,fe->size+extra);
 if(!grown){gfx_close(&fb);return;}fe->data=grown;
 for(int g=0;g<6;g++){
  unsigned off=fe->size;w32(fe->data+GFX_FONT_TABLE+slots[g]*4,off);unsigned char*record=fe->data+off;
  memset(record,0,2u+(unsigned)font.height*2u);record[0]=g<4?6:8;record[1]=2;
  const char *label=controls_pad_label(g);
  int face=!strcmp(label,"\xE2\x97\x8B")||!strcmp(label,"\xC3\x97")||!strcmp(label,"\xE2\x96\xA1")||!strcmp(label,"\xE2\x96\xB3");
  int side=face?5:7;record[0]=(unsigned char)(side+1);
  unsigned char mask[64];egg_ps5_game_prompt_mask(label,mask,side,side,1);
  int y0=font.height>8?(face?2:1):(face?1:0);
  for(int y=0;y<side;y++)for(int x=0;x<side;x++)if(mask[y*side+x]){
   int sx=x+1,sy=y+y0+1,bit=sx*2;record[2+sy*2+bit/8]|=(unsigned char)(3u<<(bit%8));
  }
  for(int y=0;y<side;y++)for(int x=0;x<side;x++)if(mask[y*side+x]){
   int bit=x*2,k=2+(y+y0)*2+bit/8;record[k]=(unsigned char)((record[k]&~(3u<<(bit%8)))|(1u<<(bit%8)));
  }
  fe->size+=2u+(unsigned)font.height*2u;
 }
 /* Staged edits retain file IDs, so file-list rebuilds cannot invalidate them. */
 for(int i=0;i<files;i++)if(in_folder(gfx_file(i),folder)){
  const GfxFile*tf=gfx_file(i);GfxBlob b=message_open(tf);size_t count;unsigned end;
  if(!text_header(&b,&count,&end,&font)){gfx_close(&b);continue;}
  Buf out={0};size_t head=16+count*4;unsigned char*zeros=calloc(head,1);int ok=zeros&&push(&out,zeros,head),changed=0;free(zeros);
  if(ok)memcpy(out.data,b.data,12);
  for(size_t j=0;j<count&&ok;j++){
   unsigned*cp;size_t n;if(!read_string(&b,j,end,&cp,&n)){ok=0;break;}
   if(out.size>UINT32_MAX){free(cp);ok=0;break;}w32(out.data+16+j*4,(uint32_t)out.size);
   int changes=rewrite(&out,&font,cp,n,slots);free(cp);unsigned char zero=(unsigned char)end;
   if(changes<0||!push(&out,&zero,1))ok=0;else changed+=changes;
  }
  if(ok&&changed&&out.size<=UINT32_MAX){
   w32(out.data+12,(uint32_t)out.size);Edit*e=stage(tf,&b);if(e){free(e->data);e->data=out.data;e->size=(uint32_t)out.size;out.data=NULL;text_count+=changed;}
  }
  free(out.data);gfx_close(&b);
 }
 gfx_close(&fb);
}
static int commit(void){
 /* Recompress and reserve all output before changing any FAT entries. A failed
  * allocation must not leave text pointing at a font that was not installed. */
 unsigned char **packed=calloc((size_t)edit_count,sizeof *packed);
 uint32_t *sizes=calloc((size_t)edit_count,sizeof *sizes);
 size_t *offsets=calloc((size_t)edit_count,sizeof *offsets);
 int ok=packed&&sizes&&offsets,written=0;size_t total=rt_rom.size;
 for(int i=0;i<edit_count&&ok;i++){
  Edit *e=&edits[i];unsigned char *data=e->data;uint32_t size=e->size;
  for(int k=0;k<e->levels;k++){
   uint32_t n;unsigned char *c=gfx_lz10_compress(data,size,&n);
   if(data!=e->data)free(data);
   data=c;if(!c){ok=0;break;}size=n;
  }
  if(!ok)break;
  packed[i]=data;sizes[i]=size;
  if(total>UINT32_MAX-511u){ok=0;break;}
  offsets[i]=(total+511u)&~(size_t)511u;
  if(size>UINT32_MAX-offsets[i]){ok=0;break;}
  total=offsets[i]+size;
 }
 if(ok){
  uint32_t fat=gfx_le32(rt_rom.data+0x48);unsigned char *rom=realloc(rt_rom.data,total);
  if(rom){
   memset(rom+rt_rom.size,255,total-rt_rom.size);rt_rom.data=rom;rt_rom.size=total;
   for(int i=0;i<edit_count;i++){
    memcpy(rom+offsets[i],packed[i],sizes[i]);
    w32(rom+fat+edits[i].file_id*8,(uint32_t)offsets[i]);
    w32(rom+fat+edits[i].file_id*8+4,(uint32_t)(offsets[i]+sizes[i]));written++;
   }
  }
 }
 for(int i=0;i<edit_count;i++){
  if(packed&&packed[i]&&packed[i]!=edits[i].data)free(packed[i]);
  free(edits[i].data);
 }
 free(packed);free(sizes);free(offsets);free(edits);edits=NULL;edit_count=0;
 return written;
}
int egg_ps5_game_prompts_init(void){
 if(!rt_rom.data||rt_rom.size<512||memcmp(rt_rom.code,"YQUE",4))return 0;
 sprite_count=text_count=0;gfx_list_files();
 sheet("icon",0);sheet("icon_fra",0);sheet("bticon",1);sheet("bticon_fra",1);
 sheet("fldicon",2);sheet("fldicon_fra",2);sheet("exicon",3);sheet("icon_bike",4);
 font_messages("msg/big/");font_messages("msg/small/");
 int expected=edit_count,written=commit();gfx_list_files();
 if(written!=expected)rt_log("PS5 game prompts: allocation failed; original assets retained");
 rt_log("PS5 game prompts: %d sprite variants, %d explicit button references, %d in-memory files; restart after rebinding",sprite_count,text_count,written);
 return written;
}
