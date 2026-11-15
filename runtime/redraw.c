/* SPDX-License-Identifier: GPL-3.0-or-later
 * Small authored-art pilot. Exact whole-object groups identify the pose;
 * frame tiles are deliberately excluded from the global native-tile hash. */
#include "redraw.h"
#include "gfx_rom.h"
#include "gfx_mod.h"
#include "sprite_hd.h"
#include "rt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define POSES 4
typedef struct Pose {
    int cell,w,h,x0,y0,scale,cols,top,seen;
    RedrawReference ref;
    uint32_t *original,*preview,*tile;
    uint32_t *representative;
    SDL_Texture *old_texture,*new_texture;
} Pose;
static Pose poses[POSES];static int count,viewer,selected;
static uint16_t canonical[16];static SDL_AtomicInt visible;
static RedrawBinding bindings[2][128];
static uint32_t rgb(SDL_Color c){return c.r|((uint32_t)c.g<<8)|((uint32_t)c.b<<16);}
static uint32_t bgr(uint16_t c){
    unsigned r=c&31,g=(c>>5)&31,b=(c>>10)&31;
    return 0xff000000u|(r<<3|r>>2)|((g<<3|g>>2)<<8)|((b<<3|b>>2)<<16);
}
uint32_t ct_redraw_color(uint32_t texel,const uint16_t*p){
    if(!texel)return 0;
    int flat=1;for(int k=3;k<=11;k++)flat&=(p[k]&0x7fff)==(p[2]&0x7fff);
    if(flat)return bgr(p[2]);
    /* Character outline entries and weapon slots are runtime-controlled.
     * Keep the illustration's RGB body colours; transfer a changed outline. */
    if((texel>>24)==254&&(p[1]&0x7fff)!=canonical[1])return bgr(p[1]);
    return texel|0xff000000u;
}
static int load_pose(int cell,const GfxBlob*cb,const GfxTiles*t,const GfxPalette*pal){
    Pose p={.cell=cell,.x0=999,.y0=999};GfxObject obj[REDRAW_OBJECTS];GfxLayout layout;
    int n=gfx_cell_objects(cb,(uint32_t)cell,obj,REDRAW_OBJECTS);
    if(!n||!gfx_cell_layout(cb,(uint32_t)cell,&layout))return 0;
    p.w=layout.w;p.h=layout.h;
    if(p.w<1||p.h<1||p.w>64||p.h>64){free(layout.tiles);return 0;}
    for(int k=0;k<n;k++){
        if(obj[k].w!=16||obj[k].h!=16||obj[k].off+128>t->size){free(layout.tiles);return 0;}
        if(obj[k].x<p.x0)p.x0=obj[k].x;
        if(obj[k].y<p.y0)p.y0=obj[k].y;
        int opaque=0;for(int b=0;b<128;b++)opaque|=t->data[obj[k].off+b];
        if(!opaque)continue; /* Empty OAM padding is not an actor identity. */
        int at=p.ref.count++;
        p.ref.object[at]=(RedrawObject){.valid=1,.x=obj[k].x,.y=obj[k].y,.flip=obj[k].hflip|obj[k].vflip<<1};
        memcpy(p.ref.object[at].bytes,t->data+obj[k].off,128);
        if(obj[k].y<p.ref.object[p.top].y)p.top=at;
    }
    if(!p.ref.count){free(layout.tiles);return 0;}
    char path[1024];snprintf(path,sizeof path,"%smods/marle-pilot/marle-%03d.png",rt_data_dir(),cell);
    SDL_Surface*raw=SDL_LoadPNG(path);if(!raw){free(layout.tiles);rt_log("redraw pilot: %s: %s",path,SDL_GetError());return 0;}
    p.scale=raw->w/p.w;
    if((p.scale!=2&&p.scale!=4&&p.scale!=8)||raw->w!=p.w*p.scale||raw->h!=p.h*p.scale){SDL_DestroySurface(raw);free(layout.tiles);rt_log("redraw pilot: cell %d has wrong canvas; original retained",cell);return 0;}
    SDL_Surface*s=SDL_ConvertSurface(raw,SDL_PIXELFORMAT_ABGR8888);SDL_DestroySurface(raw);
    if(!s){free(layout.tiles);return 0;}
    size_t pixels=(size_t)s->w*s->h;
    uint32_t*words=calloc(pixels,4);p.preview=calloc(pixels,4);p.original=calloc((size_t)p.w*p.h,4);p.representative=calloc((size_t)p.w*p.h,4);
    p.cols=(p.w+7)/8;int rows=(p.h+7)/8;p.tile=calloc((size_t)p.cols*rows,4);
    if(!words||!p.preview||!p.original||!p.representative||!p.tile){free(words);free(p.preview);free(p.original);free(p.representative);free(p.tile);SDL_DestroySurface(s);free(layout.tiles);return 0;}
    uint8_t original[64*64]={0};gfx_draw(&layout,t,original);free(layout.tiles);
    for(int k=0;k<p.w*p.h;k++)if(original[k])p.original[k]=0xff000000u|rgb(pal->c[original[k]]);
    for(int y=0;y<s->h;y++)for(int x=0;x<s->w;x++){
        uint32_t c=((uint32_t*)((uint8_t*)s->pixels+y*s->pitch))[x];
        if((c>>24)<128)continue;
        int edge=0;
        if((c&255)+((c>>8)&255)+((c>>16)&255)<288)for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){
            int xx=x+dx,yy=y+dy;
            if(xx<0||yy<0||xx>=s->w||yy>=s->h||(((uint32_t*)((uint8_t*)s->pixels+yy*s->pitch))[xx]>>24)<128)edge=1;
        }
        size_t i=(size_t)y*s->w+x;words[i]=(c&0xffffffu)|((edge?254u:255u)<<24);p.preview[i]=c|0xff000000u;
        p.representative[(y/p.scale)*p.w+x/p.scale]=words[i];
    }
    for(int ty=0;ty<rows;ty++)for(int tx=0;tx<p.cols;tx++){
        int dim=8*p.scale;uint32_t*px=calloc((size_t)dim*dim,4);if(!px)continue;
        for(int y=0;y<dim;y++)for(int x=0;x<dim;x++){
            int sx=tx*dim+x,sy=ty*dim+y;if(sx<s->w&&sy<s->h)px[y*dim+x]=words[(size_t)sy*s->w+sx];
        }
        p.tile[ty*p.cols+tx]=gfxmod_add_direct(p.scale,3,px);
    }
    free(words);SDL_DestroySurface(s);
    for(int i=0;i<p.cols*rows;i++)if(!p.tile[i]){free(p.preview);free(p.original);free(p.representative);free(p.tile);return 0;}
    poses[count++]=p;rt_log("redraw pilot: loaded Marle cell %d, %dx%d at %dx, %d nonblank original objects",cell,p.w,p.h,p.scale,p.ref.count);return 1;
}
void ct_redraw_load(void){
    char path[1024];snprintf(path,sizeof path,"%smods/marle-pilot/pilot.ini",rt_data_dir());FILE*f=fopen(path,"r");if(!f)return;
    char line[128];int enabled=0;while(fgets(line,sizeof line,f)){int v;char extra;if(sscanf(line," enabled = %d %c",&v,&extra)==1)enabled=v==1;}fclose(f);
    if(!enabled||strcmp(rt_rom.code,"YQUE")||!gfx_list_files())return;
    GfxBlob tb=gfx_open(gfx_find("Chara/Chara_0001.NCGR")),cb=gfx_open(gfx_find("Chara/Chara_0001.NCER"));GfxTiles t;GfxPalette pal;
    if(!gfx_read_tiles(&tb,&t)||t.bpp8||!gfx_read_palette(gfx_find("Chara/ObjPlt_0001.NCLR"),&pal)||pal.colors<16)goto done;
    for(int i=0;i<16;i++)canonical[i]=(pal.c[i].r>>3)|((pal.c[i].g>>3)<<5)|((pal.c[i].b>>3)<<10);
    static const int ids[]={89,90,91,211};for(int k=0;k<POSES;k++)load_pose(ids[k],&cb,&t,&pal);
    SDL_SetAtomicInt(&visible,count>0);
    rt_log("redraw pilot: %d authored poses available; all other poses use original art",count);
done:gfx_close(&tb);gfx_close(&cb);
}
int ct_redraw_count(void){return count;}
void ct_redraw_set_visible(int on){SDL_SetAtomicInt(&visible,on&&count);}
static int palette_ok(const uint16_t*p){
    int same=1,flat=1;for(int k=2;k<=11;k++){same&=(p[k]&0x7fff)==canonical[k];flat&=(p[k]&0x7fff)==(p[2]&0x7fff);}
    return same||flat;
}
int ct_redraw_prepare(int e,const RedrawObject live[128],const uint16_t palette[256]){
    memset(bindings[e],0,sizeof bindings[e]);if(!SDL_GetAtomicInt(&visible))return 0;
    int groups=0;
    for(int p=0;p<count;p++)for(int a=0;a<128;a++){
        if(bindings[e][a].action||!live[a].valid||live[a].palette<0||live[a].palette>15||!palette_ok(palette+live[a].palette*16))continue;
        int slots[REDRAW_OBJECTS];Pose*f=poses+p;if(!redraw_match(&f->ref,live,a,slots))continue;
        int used=0,draw=slots[0];for(int k=0;k<f->ref.count;k++){used|=bindings[e][slots[k]].action;if(slots[k]<draw)draw=slots[k];}
        if(used)continue;
        for(int k=0;k<f->ref.count;k++)bindings[e][slots[k]].action=1;
        bindings[e][draw]=(RedrawBinding){2,p,(live[a].x-f->ref.object[0].x+f->x0)&511,(live[a].y-f->ref.object[0].y+f->y0)&255,f->w,f->h,live[slots[f->top]].above};groups++;
        if(!f->seen){f->seen=1;rt_log("redraw pilot: matched authored Marle cell %d in engine %d",f->cell,e);}
    }return groups;
}
const RedrawBinding*ct_redraw_binding(int e,int slot){return e>=0&&e<2&&slot>=0&&slot<128&&bindings[e][slot].action?&bindings[e][slot]:NULL;}
uint32_t ct_redraw_sample(int p,int x,int y,uint32_t*texel){
    if(p<0||p>=count)return 0;
    Pose*f=poses+p;if(x<0||y<0||x>=f->w||y>=f->h)return 0;
    *texel=f->representative[y*f->w+x];return *texel?f->tile[(y/8)*f->cols+x/8]:0;
}
int ct_redraw_viewer_open(void){if(!count)return 0;viewer=1;selected=0;return 1;}
int ct_redraw_viewer_active(void){return viewer;}
void ct_redraw_viewer_close(void){viewer=0;}
void ct_redraw_viewer_step(int d){if(count)selected=(selected+count+(d<0?-1:1))%count;}
const char*ct_redraw_viewer_title(void){static char title[64];snprintf(title,sizeof title,"Marle cell %03d - art pilot",count?poses[selected].cell:0);return title;}
void ct_redraw_viewer_draw(SDL_Renderer*ren,const SDL_FRect*v){
    if(!viewer||!count)return;
    Pose*f=poses+selected;
    if(!f->old_texture){
        f->old_texture=SDL_CreateTexture(ren,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STATIC,f->w,f->h);
        f->new_texture=SDL_CreateTexture(ren,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STATIC,f->w*f->scale,f->h*f->scale);
        if(!f->old_texture||!f->new_texture){if(f->old_texture)SDL_DestroyTexture(f->old_texture);if(f->new_texture)SDL_DestroyTexture(f->new_texture);f->old_texture=f->new_texture=NULL;return;}
        SDL_UpdateTexture(f->old_texture,NULL,f->original,f->w*4);SDL_UpdateTexture(f->new_texture,NULL,f->preview,f->w*f->scale*4);
        SDL_SetTextureBlendMode(f->old_texture,SDL_BLENDMODE_BLEND);SDL_SetTextureBlendMode(f->new_texture,SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(f->old_texture,SDL_SCALEMODE_NEAREST);SDL_SetTextureScaleMode(f->new_texture,SDL_SCALEMODE_NEAREST);
    }
    float sx=v->w/256,sy=v->h/192,k=(sx<sy?sx:sy)*1.55f;
    SDL_SetRenderDrawColor(ren,24,29,35,255);SDL_RenderFillRect(ren,&(SDL_FRect){v->x+v->w*.04f,v->y+sy*48,v->w*.92f,sy*106});
    for(int side=0;side<2;side++){
        float origin=v->x+v->w*(side?.74f:.26f),baseline=v->y+sy*148;
        SDL_FRect r={origin+f->x0*k,baseline+f->y0*k,f->w*k,f->h*k};
        SDL_RenderTexture(ren,side?f->new_texture:f->old_texture,NULL,&r);
    }
}
