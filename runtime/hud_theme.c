/* SPDX-License-Identifier: GPL-3.0-or-later
 * Existing battle-HUD appearance layer. Snapshots and input remain owned by
 * battle_hud.c; only presentation is configurable. No runtime game writes. */
#include "hud_theme.h"
#include "mod_json.h"
#include "ui_font.h"
#include <limits.h>
#define RGB(r,g,b) (0xff000000u|(uint32_t)(r)|(uint32_t)(g)<<8|(uint32_t)(b)<<16)
static const char*asset_names[CT_THEME_IMAGES]={"background.png","card.png","frame.png","overlay.png","portraits/frame.png","font.png","bars/track.png","bars/hp.png","bars/mp.png","bars/atb.png","bars/atb_ready.png","icons/attack.png","icons/tech.png","icons/item.png","portraits/crono.png","portraits/marle.png","portraits/lucca.png","portraits/robo.png","portraits/frog.png","portraits/ayla.png","portraits/magus.png",NULL,NULL};
static const char*colour_names[TC_COLOURS]={"background","text","labels","border","divider","portrait_border","selected","bar_track","hp","mp","atb","atb_ready","low_hp","outline","ko"};
const char*ct_hud_theme_asset_name(int i){return i>=0&&i<CT_THEME_IMAGES?asset_names[i]:NULL;}
void ct_hud_theme_defaults(CtHudThemeConfig*c){
 memset(c,0,sizeof *c);c->width=480;c->height=80;c->scale=3;c->border=1;c->opacity=255;c->max_width_percent=94;c->command_gap=2;
 uint32_t colours[]={RGB(20,26,31),RGB(237,241,245),RGB(195,206,216),RGB(156,164,172),RGB(71,83,94),RGB(137,146,155),RGB(239,242,245),RGB(24,30,36),RGB(105,164,119),RGB(115,151,181),RGB(139,165,181),RGB(209,224,231),RGB(255,187,102),RGB(0,0,0),RGB(143,151,159)};memcpy(c->colour,colours,sizeof colours);
 c->portrait=(CtThemeRect){6,13,48,48};c->hp_bar=(CtThemeRect){60,35,92,4};c->mp_bar=(CtThemeRect){60,55,92,4};c->atb_bar=(CtThemeRect){34,68,118,4};
 c->name=(CtThemePoint){66,6};c->hp_label=(CtThemePoint){60,25};c->hp_value=(CtThemePoint){80,25};c->mp_label=(CtThemePoint){60,45};c->mp_value=(CtThemePoint){80,45};c->atb_label=(CtThemePoint){6,66};c->name_size=3;c->value_size=c->label_size=2;
 c->show_faces=c->show_names=c->show_labels=c->show_numbers=c->show_hp=c->show_mp=c->show_atb=c->show_dividers=c->show_selection=c->show_commands=1;
 c->font_columns=16;c->font_rows=6;c->font_first=32;c->font_count=96;c->font_tint=1;
 const char*labels[]={"HP","MP","ATB","Attack","Tech","Item"};for(int i=0;i<6;i++)snprintf(c->labels[i],sizeof c->labels[i],"%s",labels[i]);snprintf(c->name_label,sizeof c->name_label,"Original + PNGs");
 for(int i=0;i<CT_THEME_IMAGES;i++)if(asset_names[i])snprintf(c->files[i],CT_THEME_PATH,"%s",asset_names[i]);
}
static int field_int(const MjReader*r,int obj,const char*key,int lo,int hi,int*out){int k=mj_field(r,obj,key);return k==-1||mj_signed(r,k,lo,hi,out);}
static int field_bool(const MjReader*r,int obj,const char*key,int*out){return mj_boolean(r,mj_field(r,obj,key),out);}
static int field_string(const MjReader*r,int obj,const char*key,char*out,size_t n){int k=mj_field(r,obj,key);return k==-1||mj_string(r,k,out,n);}
static int field_array(const MjReader*r,int obj,const char*key,int*out,int n,int hi){int k=mj_field(r,obj,key);return k==-1||mj_array_int(r,k,out,n,0,hi);}
static int file_field(const MjReader*r,int obj,const char*key,char out[CT_THEME_PATH]){int k=mj_field(r,obj,key);if(k==-1)return 1;if(k>=0&&r->t[k].type=='n'){out[0]=0;return 1;}return mj_path(r,k,out);}
static int object(const MjReader*r,int parent,const char*key){int k=mj_field(r,parent,key);if(k>=0&&r->t[k].type!='{')return -2;return k;}
static int rect_valid(CtThemeRect q,int w,int h){return q.x>=0&&q.y>=0&&q.w>0&&q.h>0&&q.x<=w-q.w&&q.y<=h-q.h;}
int ct_hud_theme_parse(const char*json,size_t length,CtHudThemeConfig*out,char*error,size_t capacity){
 MjReader r;const char*why="Invalid theme JSON";if(!out||!mj_open(&r,json,length))goto fail;
 CtHudThemeConfig c;ct_hud_theme_defaults(&c);int schema,k;
 if(!mj_integer(&r,mj_field(&r,0,"schema"),1,1,&schema)){why="Unsupported HUD theme schema";goto bad;}
 if(!field_string(&r,0,"name",c.name_label,sizeof c.name_label)||!field_int(&r,0,"pixel_scale",1,4,&c.scale)||!field_int(&r,0,"opacity",0,255,&c.opacity))goto bad;
 int canvas[2]={c.width,c.height};if(!field_array(&r,0,"canvas",canvas,2,1200))goto bad;c.width=canvas[0];c.height=canvas[1];
 if(c.width<480||c.width>1200||c.height<80||c.height>240||(uint64_t)c.width*c.height*c.scale*c.scale>2000000){why="HUD canvas exceeds supported bounds";goto bad;}
 k=object(&r,0,"colors");if(k==-2)goto bad;if(k>=0)for(int i=0;i<TC_COLOURS;i++){int at=mj_field(&r,k,colour_names[i]);if(at!=-1){if(!mj_colour(&r,at,c.colour+i)){why="Invalid #RRGGBB or #RRGGBBAA theme colour";goto bad;}if(i==TC_BG)c.background_colour_set=1;}}
 k=object(&r,0,"layout");if(k==-2)goto bad;if(k>=0){
  if(!field_int(&r,k,"gap",0,40,&c.gap)||!field_int(&r,k,"padding",0,40,&c.padding)||!field_array(&r,k,"portrait",&c.portrait.x,4,1200)||!field_array(&r,k,"hp_bar",&c.hp_bar.x,4,1200)||!field_array(&r,k,"mp_bar",&c.mp_bar.x,4,1200)||!field_array(&r,k,"atb_bar",&c.atb_bar.x,4,1200))goto bad;
  const char*keys[]={"name","hp_label","hp_value","mp_label","mp_value","atb_label"};CtThemePoint*points[]={&c.name,&c.hp_label,&c.hp_value,&c.mp_label,&c.mp_value,&c.atb_label};for(int i=0;i<6;i++)if(!field_array(&r,k,keys[i],&points[i]->x,2,1200))goto bad;
 }
 int card=(c.width-2*c.padding-2*c.gap)/3;
 if(!rect_valid(c.portrait,card,c.height)||!rect_valid(c.hp_bar,card,c.height)||!rect_valid(c.mp_bar,card,c.height)||!rect_valid(c.atb_bar,card,c.height)){why="Theme element lies outside its party card";goto bad;}
 CtThemePoint points[]={c.name,c.hp_label,c.hp_value,c.mp_label,c.mp_value,c.atb_label};for(int i=0;i<6;i++)if(points[i].x<0||points[i].x>=card||points[i].y<0||points[i].y>=c.height){why="Theme text position lies outside its party card";goto bad;}
 k=object(&r,0,"text");if(k==-2)goto bad;if(k>=0){if(!field_int(&r,k,"name_size",2,8,&c.name_size)||!field_int(&r,k,"value_size",2,8,&c.value_size)||!field_int(&r,k,"label_size",2,8,&c.label_size))goto bad;const char*keys[]={"hp","mp","atb","attack","tech","item"};for(int i=0;i<6;i++)if(!field_string(&r,k,keys[i],c.labels[i],sizeof c.labels[i]))goto bad;}
 k=object(&r,0,"show");if(k==-2)goto bad;if(k>=0){const char*keys[]={"portraits","names","labels","numbers","hp","mp","atb","dividers","selection","commands"};int*flags[]={&c.show_faces,&c.show_names,&c.show_labels,&c.show_numbers,&c.show_hp,&c.show_mp,&c.show_atb,&c.show_dividers,&c.show_selection,&c.show_commands};for(int i=0;i<10;i++)if(!field_bool(&r,k,keys[i],flags[i]))goto bad;}
 k=object(&r,0,"style");if(k==-2)goto bad;if(k>=0){if(!field_int(&r,k,"border_width",0,8,&c.border)||!field_int(&r,k,"grain",0,24,&c.grain)||!field_bool(&r,k,"clocks",&c.clocks)||!field_bool(&r,k,"linear_sampling",&c.linear)||!field_int(&r,k,"background_tint",0,255,&c.background_tint))goto bad;
  char fit[16]="cover",mode[16]="auto";if(!field_string(&r,k,"portrait_fit",fit,sizeof fit)||!field_string(&r,k,"background_mode",mode,sizeof mode))goto bad;if(!strcmp(fit,"contain"))c.portrait_fit=1;else if(strcmp(fit,"cover"))goto bad;
  if(!strcmp(mode,"tile"))c.background_mode=1;else if(!strcmp(mode,"stretch"))c.background_mode=2;else if(!strcmp(mode,"window"))c.background_mode=3;else if(strcmp(mode,"auto"))goto bad;
 }
 k=object(&r,0,"font");if(k==-2)goto bad;if(k>=0)if(!field_int(&r,k,"columns",1,64,&c.font_columns)||!field_int(&r,k,"rows",1,64,&c.font_rows)||!field_int(&r,k,"first",0,127,&c.font_first)||!field_int(&r,k,"count",1,128,&c.font_count)||!field_bool(&r,k,"tint",&c.font_tint))goto bad;
 if(c.font_count>c.font_columns*c.font_rows||c.font_first+c.font_count>128)goto bad;
 k=object(&r,0,"anchor");if(k==-2)goto bad;if(k>=0)if(!field_int(&r,k,"max_width_percent",40,100,&c.max_width_percent)||!field_int(&r,k,"offset_x",-256,256,&c.offset_x)||!field_int(&r,k,"offset_y",-192,192,&c.offset_y)||!field_int(&r,k,"command_gap",0,32,&c.command_gap))goto bad;
 k=object(&r,0,"assets");if(k==-2)goto bad;if(k>=0){const char*keys[]={"background","card","frame","overlay","portrait_frame","font","bar_track","hp_bar","mp_bar","atb_bar","atb_ready","attack","tech","item","crono","marle","lucca","robo","frog","ayla","magus"};for(int i=0;i<21;i++)if(!file_field(&r,k,keys[i],c.files[i])){why="Theme image path must stay inside its folder and end in .png";goto bad;}}
 k=object(&r,0,"names");if(k==-2)goto bad;if(k>=0){const char*keys[]={"crono","marle","lucca","robo","frog","ayla","magus"};for(int i=0;i<7;i++)if(!field_string(&r,k,keys[i],c.names[i],sizeof c.names[i]))goto bad;}
 *out=c;mj_close(&r);if(error&&capacity)error[0]=0;return 1;
bad:mj_close(&r);
fail:if(error&&capacity)snprintf(error,capacity,"%s",why);return 0;
}
static int join(char*out,size_t n,const char*root,const char*file){if(!root||!root[0]||!file||!file[0])return 0;int k=snprintf(out,n,"%s%s%s",root,root[strlen(root)-1]=='/'?"":"/",file);return k>=0&&(size_t)k<n;}
static void note(CtThemeLog log,const char*action,const char*path){if(log){char text[1200];snprintf(text,sizeof text,"HUD theme: %s: %s; original asset retained",action,path);log(text);}}
static uint32_t be32(const uint8_t*p){return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|p[3];}
static SDL_Surface*load_image(const char*path,size_t used,CtThemeLog log){
 FILE*f=fopen(path,"rb");if(!f)return NULL;uint8_t head[24];int good=fread(head,1,sizeof head,f)==sizeof head&&!memcmp(head,"\211PNG\r\n\032\n",8)&&!memcmp(head+12,"IHDR",4);unsigned w=good?be32(head+16):0,h=good?be32(head+20):0;
 if(!good||!w||!h||w>2048||h>2048||(uint64_t)w*h*4+used>32u*1024*1024||fseek(f,0,SEEK_END)!=0){fclose(f);note(log,"invalid/oversized PNG",path);return NULL;}
 long bytes=ftell(f);if(bytes<24||bytes>16*1024*1024||fseek(f,0,SEEK_SET)!=0){fclose(f);note(log,"invalid PNG size",path);return NULL;}
 uint8_t*data=malloc((size_t)bytes);good=data&&fread(data,1,(size_t)bytes,f)==(size_t)bytes;if(fclose(f))good=0;
 SDL_Surface*raw=NULL;if(good){SDL_IOStream*io=SDL_IOFromConstMem(data,(size_t)bytes);if(io)raw=SDL_LoadPNG_IO(io,true);}free(data);
 if(!raw){note(log,"unreadable PNG",path);return NULL;}SDL_Surface*s=SDL_ConvertSurface(raw,SDL_PIXELFORMAT_ABGR8888);SDL_DestroySurface(raw);if(!s){note(log,"unsupported PNG",path);return NULL;}return s;
}
CtHudTheme*ct_hud_theme_load(const char*root,const char*fallback,char*error,size_t capacity,CtThemeLog log){
 if(error&&capacity)error[0]=0;CtHudTheme*t=calloc(1,sizeof *t);if(!t){if(error&&capacity)snprintf(error,capacity,"Theme allocation failed");return NULL;}ct_hud_theme_defaults(&t->cfg);
 char path[1200];int has_json=0;if(join(path,sizeof path,root,"theme.json")){
  FILE*f=fopen(path,"rb");if(f){has_json=1;if(fseek(f,0,SEEK_END)!=0){fclose(f);goto io_error;}long n=ftell(f);if(n<=0||n>512*1024||fseek(f,0,SEEK_SET)!=0){fclose(f);goto io_error;}
   char*json=malloc((size_t)n);int good=json&&fread(json,1,(size_t)n,f)==(size_t)n;if(fclose(f))good=0;if(!good){free(json);goto io_error;}good=ct_hud_theme_parse(json,(size_t)n,&t->cfg,error,capacity);free(json);if(!good){ct_hud_theme_free(t);return NULL;}
  }
 }
 for(int i=0;i<CT_THEME_IMAGES;i++)if(t->cfg.files[i][0]){
  SDL_Surface*s=NULL;if(join(path,sizeof path,root,t->cfg.files[i]))s=load_image(path,t->asset_bytes,log);
  if(!s&&fallback&&(!root||strcmp(root,fallback))&&asset_names[i]&&join(path,sizeof path,fallback,asset_names[i]))s=load_image(path,t->asset_bytes,log);
  if(!s)continue;t->image[i].surface=s;t->asset_bytes+=(size_t)s->w*s->h*4;t->overrides++;
 }
 if(t->image[TH_FONT].surface){SDL_Surface*s=t->image[TH_FONT].surface;if(s->w%t->cfg.font_columns||s->h%t->cfg.font_rows){note(log,"font PNG does not match glyph grid","font.png");t->asset_bytes-=(size_t)s->w*s->h*4;SDL_DestroySurface(s);t->image[TH_FONT].surface=NULL;t->overrides--;}}
 if(!has_json&&!t->overrides){ct_hud_theme_free(t);return NULL;}
 t->pixel_w=t->cfg.width*t->cfg.scale;t->pixel_h=t->cfg.height*t->cfg.scale;t->command_w=CT_CMD_W*t->cfg.scale;t->command_h=CT_CMD_H*t->cfg.scale;
 t->pixels=calloc((size_t)t->pixel_w*t->pixel_h,4);t->commands=calloc((size_t)t->command_w*t->command_h,4);
 if(!t->pixels||!t->commands)goto io_error;
 return t;
io_error:if(error&&capacity)snprintf(error,capacity,"Could not read or allocate HUD theme");ct_hud_theme_free(t);return NULL;
}
void ct_hud_theme_free(CtHudTheme*t){if(!t)return;for(int i=0;i<CT_THEME_IMAGES;i++)SDL_DestroySurface(t->image[i].surface);SDL_DestroySurface(t->surface);SDL_DestroySurface(t->command_surface);free(t->pixels);free(t->commands);free(t);}
/* All primitives clip to the output canvas/card, including user-supplied
 * layouts. The entire RGBA canvas is rebuilt each frame: no stale patches. */
typedef struct Canvas {uint32_t*p;int w,h,scale,clip_left,clip_right;} Canvas;
static uint32_t over(uint32_t dest,uint32_t src){
 unsigned a=src>>24,b=dest>>24;if(!a)return dest;if(a==255)return src;unsigned out=a+b*(255-a)/255;if(!out)return 0;uint32_t c=out<<24;
 for(int k=0;k<24;k+=8){unsigned x=(src>>k)&255,y=(dest>>k)&255;c|=((x*a*255+y*b*(255-a))/(out*255))<<k;}return c;
}
static void pixel(Canvas*c,int x,int y,uint32_t colour){if(y>=0&&y<c->h&&x>=c->clip_left&&x<c->clip_right&&x>=0&&x<c->w)c->p[(size_t)y*c->w+x]=over(c->p[(size_t)y*c->w+x],colour);}
static void box(Canvas*c,int x,int y,int w,int h,uint32_t colour){
 int x0=x*c->scale,y0=y*c->scale,x1=(x+w)*c->scale,y1=(y+h)*c->scale;if(x0<c->clip_left)x0=c->clip_left;if(x1>c->clip_right)x1=c->clip_right;if(y0<0)y0=0;if(y1>c->h)y1=c->h;
 for(int yy=y0;yy<y1;yy++)for(int xx=x0;xx<x1;xx++)pixel(c,xx,yy,colour);
}
static void stroke(Canvas*c,CtThemeRect r,int n,uint32_t colour){if(n<=0)return;box(c,r.x,r.y,r.w,n,colour);box(c,r.x,r.y+r.h-n,r.w,n,colour);box(c,r.x,r.y,n,r.h,colour);box(c,r.x+r.w-n,r.y,n,r.h,colour);}
static uint32_t shade(uint32_t c,int delta){uint32_t out=c&0xff000000u;for(int k=0;k<24;k+=8){int v=(int)((c>>k)&255)+delta;out|=(uint32_t)(v<0?0:v>255?255:v)<<k;}return out;}
static uint32_t sample(const uint32_t*p,int w,int h,int pitch,float x,float y,int linear){
 if(!linear){int xx=(int)(x+.5f),yy=(int)(y+.5f);xx=xx<0?0:xx>=w?w-1:xx;yy=yy<0?0:yy>=h?h-1:yy;return p[(size_t)yy*pitch+xx];}
 int ix=(int)x,iy=(int)y;if(x<0&&x!=ix)ix--;if(y<0&&y!=iy)iy--;float fx=x-ix,fy=y-iy;float a=0,channel[3]={0};
 for(int dy=0;dy<2;dy++)for(int dx=0;dx<2;dx++){int xx=ix+dx,yy=iy+dy;xx=xx<0?0:xx>=w?w-1:xx;yy=yy<0?0:yy>=h?h-1:yy;uint32_t c=p[(size_t)yy*pitch+xx];float weight=(dx?fx:1-fx)*(dy?fy:1-fy),alpha=(float)(c>>24)*weight;a+=alpha;for(int k=0;k<3;k++)channel[k]+=((c>>(k*8))&255)*alpha;}
 if(a<.5f)return 0;uint32_t out=(uint32_t)(a+.5f)<<24;for(int k=0;k<3;k++)out|=(uint32_t)(channel[k]/a+.5f)<<(k*8);return out;
}
static void blit_pixels(Canvas*c,CtThemeRect r,const uint32_t*p,int iw,int ih,int pitch,int fit,int linear,int grey){
 if(!p||!r.w||!r.h)return;float scale_x=(float)r.w/iw,scale_y=(float)r.h/ih;
 if(fit>=0){float scale=fit?(scale_x<scale_y?scale_x:scale_y):(scale_x>scale_y?scale_x:scale_y);scale_x=scale_y=scale;}
 float dx=r.x+(r.w-iw*scale_x)/2,dy=r.y+(r.h-ih*scale_y)/2;
 for(int y=r.y*c->scale;y<(r.y+r.h)*c->scale;y++)for(int x=r.x*c->scale;x<(r.x+r.w)*c->scale;x++){
  float sx=((x+.5f)/c->scale-dx)/scale_x-.5f,sy=((y+.5f)/c->scale-dy)/scale_y-.5f;if(fit==1&&(sx<-.5f||sy<-.5f||sx>=iw-.5f||sy>=ih-.5f))continue;
  uint32_t word=sample(p,iw,ih,pitch,sx,sy,linear);if(grey){unsigned v=((word&255)+((word>>8)&255)+((word>>16)&255))/3;word=(word&0xff000000u)|v|v<<8|v<<16;}pixel(c,x,y,word);
 }
}
static void image(Canvas*c,CtThemeRect r,SDL_Surface*s,int fit,int linear,int grey){if(s)blit_pixels(c,r,s->pixels,s->w,s->h,s->pitch/4,fit,linear,grey);}
static int text_width(const char*s,int size){return ui_text_width(s)*size/2;}
static void text(Canvas*c,const CtHudTheme*t,int x,int y,const char*s,uint32_t colour,int size){
 if(!s||!s[0])return;SDL_Surface*font=t->image[TH_FONT].surface;
 if(!font){uint32_t glyphs[256*16]={0};ui_draw_text_in(glyphs,256,s,0,0,colour);int w=ui_text_width(s)+1;if(w>256)w=256;
  for(int yy=0;yy<9*size*c->scale/2;yy++)for(int xx=0;xx<w*size*c->scale/2;xx++){uint32_t v=glyphs[(yy*2/(size*c->scale))*256+xx*2/(size*c->scale)];if(v)pixel(c,x*c->scale+xx,y*c->scale+yy,v);}return;
 }
 int fw=font->w/t->cfg.font_columns,fh=font->h/t->cfg.font_rows,advance=6*size*c->scale/2,gheight=9*size*c->scale/2;
 for(int n=0;s[n];n++){int ch=(unsigned char)s[n]-t->cfg.font_first;if(ch<0||ch>=t->cfg.font_count)continue;int ox=(ch%t->cfg.font_columns)*fw,oy=(ch/t->cfg.font_columns)*fh;
  for(int yy=0;yy<gheight;yy++)for(int xx=0;xx<advance;xx++){int sx=ox+xx*fw/advance,sy=oy+yy*fh/gheight;uint32_t v=((uint32_t*)((uint8_t*)font->pixels+sy*font->pitch))[sx];if(t->cfg.font_tint)v=(v&0xff000000u)|(colour&0xffffffu);pixel(c,x*c->scale+n*advance+xx,y*c->scale+yy,v);}
 }
}
static void clock_corner(Canvas*c,int x,int y,uint32_t colour){
 for(int yy=0;yy<9;yy++)for(int xx=0;xx<9;xx++){int d=xx*xx+yy*yy;if(d>=49&&d<=65)box(c,x+xx,y+yy,1,1,colour);}box(c,x,y,1,7,colour);box(c,x,y,7,1,colour);box(c,x+2,y+2,1,3,colour);box(c,x+2,y+4,3,1,colour);
}
static void background(Canvas*c,const CtHudTheme*t,const uint32_t*skin){
 const CtHudThemeConfig*f=&t->cfg;SDL_Surface*img=t->image[TH_BG].surface;
 if(f->background_colour_set||(!img&&!skin))box(c,0,0,f->width,f->height,f->colour[TC_BG]);
 if(!img&&skin&&!f->background_colour_set){
  for(int y=0;y<c->h;y++)for(int x=0;x<c->w;x++){int xx=x/c->scale,yy=y/c->scale;
   int sx=xx<16?xx/2:xx>=f->width-16?80-(f->width-xx+1)/2:8+(xx-16)/2%64;
   int sy=yy<16?yy/2:yy>=f->height-16?32-(f->height-yy+1)/2:8+(yy-16)*16/(f->height-32);
   uint32_t word=skin[sy*80+sx];pixel(c,x,y,(word>>24)?word:f->colour[TC_BG]);
  }
 }else if(img){
  int mode=f->background_mode;if(!mode&&img->w%80==0&&img->h%32==0&&img->w/80==img->h/32)mode=3;
  if(mode==1){for(int y=0;y<c->h;y++)for(int x=0;x<c->w;x++)pixel(c,x,y,((uint32_t*)((uint8_t*)img->pixels+(y%img->h)*img->pitch))[x%img->w]);}
  else if(mode==3){for(int y=0;y<c->h;y++)for(int x=0;x<c->w;x++){int xx=x/c->scale,yy=y/c->scale;int sx=xx<16?xx/2:xx>=f->width-16?80-(f->width-xx+1)/2:8+(xx-16)/2%64;int sy=yy<16?yy/2:yy>=f->height-16?32-(f->height-yy+1)/2:8+(yy-16)*16/(f->height-32);pixel(c,x,y,sample(img->pixels,img->w,img->h,img->pitch/4,(sx+.5f)*img->w/80-.5f,(sy+.5f)*img->h/32-.5f,f->linear));}}
  else image(c,(CtThemeRect){0,0,f->width,f->height},img,-1,f->linear,0);
 }
 if(f->background_tint){uint32_t tint=(f->colour[TC_BG]&0xffffffu)|(unsigned)f->background_tint<<24;box(c,0,0,f->width,f->height,tint);}
 if(f->grain)for(int y=0;y<c->h;y++)for(int x=0;x<c->w;x++){unsigned h=(unsigned)(x/c->scale)*374761393u+(unsigned)(y/c->scale)*668265263u;h=(h^(h>>13))*1274126177u;int delta=(int)((h^(h>>16))%(unsigned)(f->grain*2+1))-f->grain;c->p[(size_t)y*c->w+x]=shade(c->p[(size_t)y*c->w+x],delta);}
}
static void bar(Canvas*c,const CtHudTheme*t,CtThemeRect r,unsigned now,unsigned maximum,int asset,uint32_t colour){
 box(c,r.x,r.y,r.w,r.h,t->cfg.colour[TC_TRACK]);image(c,r,t->image[TH_TRACK].surface,-1,t->cfg.linear,0);stroke(c,r,1,t->cfg.colour[TC_BORDER]);
 int inner=r.w-2,fill=maximum?(int)((uint64_t)(now>maximum?maximum:now)*(unsigned)(inner>0?inner:0)/maximum):0;
 if(fill>0&&r.h>2){CtThemeRect inside={r.x+1,r.y+1,inner,r.h-2};int old=c->clip_right,end=(inside.x+fill)*c->scale;if(end<c->clip_right)c->clip_right=end;
  if(t->image[asset].surface)image(c,inside,t->image[asset].surface,-1,t->cfg.linear,0);else{box(c,inside.x,inside.y,inside.w,inside.h,colour);box(c,inside.x,inside.y,inside.w,1,shade(colour,15));}c->clip_right=old;
 }
}
const uint32_t*ct_hud_theme_draw(CtHudTheme*t,const CtHudState*s,const uint32_t faces[7][48*48],const uint32_t*skin){
 if(!t||!s)return NULL;memset(t->pixels,0,(size_t)t->pixel_w*t->pixel_h*4);Canvas c={t->pixels,t->pixel_w,t->pixel_h,t->cfg.scale,0,t->pixel_w};CtHudThemeConfig*f=&t->cfg;background(&c,t,skin);
 int card=(f->width-2*f->padding-2*f->gap)/3;
 for(int i=0;i<3;i++){
  int x=f->padding+i*(card+f->gap);c.clip_left=x*c.scale;c.clip_right=(x+card)*c.scale;
  image(&c,(CtThemeRect){x,0,card,f->height},t->image[TH_CARD].surface,-1,f->linear,0);
  if(i&&f->show_dividers)box(&c,x,7,1,f->height-14,f->colour[TC_DIVIDER]);
  const CtHudMember*m=s->member+i;if(!m->present||m->character<0||m->character>=7)continue;
  uint32_t ink=m->hp?f->colour[TC_TEXT]:f->colour[TC_KO];
  if(f->show_faces){CtThemeRect face=f->portrait;face.x+=x;box(&c,face.x-1,face.y-1,face.w+2,face.h+2,f->colour[TC_TRACK]);stroke(&c,(CtThemeRect){face.x-1,face.y-1,face.w+2,face.h+2},1,f->show_selection&&s->selected==i?f->colour[TC_SELECTED]:f->colour[TC_FACE_BORDER]);SDL_Surface*custom=t->image[TH_CRONO+m->character].surface;
   if(custom)image(&c,face,custom,f->portrait_fit,f->linear,!m->hp);else blit_pixels(&c,face,faces[m->character],48,48,48,f->portrait_fit,f->linear,!m->hp);
   image(&c,(CtThemeRect){face.x-1,face.y-1,face.w+2,face.h+2},t->image[TH_PORTRAIT_FRAME].surface,-1,f->linear,0);if(f->clocks)clock_corner(&c,face.x,face.y+face.h-9,f->colour[TC_BORDER]);
  }
  if(f->show_selection&&s->selected==i&&m->hp)text(&c,t,x+f->name.x-9,f->name.y,">",f->colour[TC_SELECTED],2);
  if(f->show_names)text(&c,t,x+f->name.x,f->name.y,f->names[m->character][0]?f->names[m->character]:m->name,ink,f->name_size);
  char value[32];int low=m->max_hp&&(uint64_t)m->hp*4<=m->max_hp;
  if(f->show_hp){if(f->show_labels)text(&c,t,x+f->hp_label.x,f->hp_label.y,f->labels[0],f->colour[TC_LABEL],f->label_size);snprintf(value,sizeof value,"%u/%u",m->hp,m->max_hp);if(f->show_numbers)text(&c,t,x+f->hp_value.x,f->hp_value.y,value,low?f->colour[TC_LOW_HP]:ink,f->value_size);CtThemeRect r=f->hp_bar;r.x+=x;bar(&c,t,r,m->hp,m->max_hp,TH_HP,low?f->colour[TC_LOW_HP]:f->colour[TC_HP]);}
  if(f->show_mp){if(f->show_labels)text(&c,t,x+f->mp_label.x,f->mp_label.y,f->labels[1],f->colour[TC_LABEL],f->label_size);snprintf(value,sizeof value,"%u/%u",m->mp,m->max_mp);if(f->show_numbers)text(&c,t,x+f->mp_value.x,f->mp_value.y,value,ink,f->value_size);CtThemeRect r=f->mp_bar;r.x+=x;bar(&c,t,r,m->mp,m->max_mp,TH_MP,f->colour[TC_MP]);}
  if(f->show_atb){if(f->show_labels)text(&c,t,x+f->atb_label.x,f->atb_label.y,f->labels[2],f->colour[TC_LABEL],f->label_size);CtThemeRect r=f->atb_bar;r.x+=x;int ready=m->atb_max&&m->atb>=m->atb_max;bar(&c,t,r,m->atb,m->atb_max,ready?TH_READY:TH_ATB,f->colour[ready?TC_READY:TC_ATB]);}
 }
 c.clip_left=0;c.clip_right=c.w;stroke(&c,(CtThemeRect){0,0,f->width,f->height},f->border,f->colour[TC_BORDER]);image(&c,(CtThemeRect){0,0,f->width,f->height},t->image[TH_FRAME].surface,-1,f->linear,0);if(f->clocks)clock_corner(&c,f->width-10,1,f->colour[TC_BORDER]);image(&c,(CtThemeRect){0,0,f->width,f->height},t->image[TH_OVERLAY].surface,-1,f->linear,0);
 if(f->opacity!=255)for(size_t i=0,n=(size_t)c.w*c.h;i<n;i++)t->pixels[i]=(t->pixels[i]&0xffffffu)|((t->pixels[i]>>24)*(unsigned)f->opacity/255)<<24;
 return t->pixels;
}
const uint32_t*ct_hud_theme_commands(CtHudTheme*t,int selected,const uint32_t icons[3][32*32]){
 if(!t)return NULL;memset(t->commands,0,(size_t)t->command_w*t->command_h*4);if(!t->cfg.show_commands)return t->commands;Canvas c={t->commands,t->command_w,t->command_h,t->cfg.scale,0,t->command_w};
 for(int i=0;i<3;i++){
  uint32_t mini[32*32];SDL_Surface*custom=t->image[TH_ATTACK+i].surface;
  for(int y=0;y<32;y++)for(int x=0;x<32;x++)mini[y*32+x]=custom?sample(custom->pixels,custom->w,custom->h,custom->pitch/4,(x+.5f)*custom->w/32-.5f,(y+.5f)*custom->h/32-.5f,0):icons[i][y*32+x];
  for(int y=-3;y<35;y++)for(int x=-3;x<35;x++)if(!(y>=0&&y<32&&x>=0&&x<32&&(mini[y*32+x]>>24))){int outer=0,inner=0;for(int dy=-3;dy<=3;dy++)for(int dx=-3;dx<=3;dx++){int xx=x+dx,yy=y+dy;if(dx*dx+dy*dy>9||xx<0||yy<0||xx>=32||yy>=32||!(mini[yy*32+xx]>>24))continue;outer=1;if(dx*dx+dy*dy<=2)inner=1;}if(outer)box(&c,i*80+24+x,3+y,1,1,inner?t->cfg.colour[TC_TEXT]:t->cfg.colour[TC_OUTLINE]);}
  CtThemeRect r={i*80+24,3,32,32};if(custom)image(&c,r,custom,-1,t->cfg.linear,0);else blit_pixels(&c,r,icons[i],32,32,32,-1,0,0);
  uint32_t colour=i==selected?t->cfg.colour[TC_SELECTED]:t->cfg.colour[TC_TEXT];if(i==selected)box(&c,i*80+24,35,32,2,colour);
  text(&c,t,i*80+(80-text_width(t->cfg.labels[3+i],3))/2,38,t->cfg.labels[3+i],colour,3);
 }
 return t->commands;
}
