/* SPDX-License-Identifier: GPL-3.0-or-later
 * Small authored-art pilot. Exact whole-object groups identify the pose;
 * frame tiles are deliberately excluded from the global native-tile hash. */
#include "redraw.h"
#include "redraw_manifest.h"
#include "gfx_rom.h"
#include "gfx_mod.h"
#include "sprite_hd.h"
#include "rt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define POSES 4096
typedef struct Pose {
    int cell,character,module,w,h,x0,y0,scale,cols,top,seen;
    RedrawReference ref;
    uint32_t *original,*preview,*tile;
    uint32_t *representative;
    SDL_Texture *old_texture,*new_texture;
} Pose;
static Pose poses[POSES];static int count,viewer,selected;
static const char*characters[]={"Crono","Marle","Lucca","Robo","Frog","Ayla","Magus"};
static uint16_t canonical[7][16];static SDL_AtomicInt visible,gameplay[CT_REDRAW_MODULES];
static RedrawBinding bindings[2][128];
static uint32_t rgb(SDL_Color c){return c.r|((uint32_t)c.g<<8)|((uint32_t)c.b<<16);}
static uint32_t bgr(uint16_t c){
    unsigned r=c&31,g=(c>>5)&31,b=(c>>10)&31;
    return 0xff000000u|(r<<3|r>>2)|((g<<3|g>>2)<<8)|((b<<3|b>>2)<<16);
}
uint32_t ct_redraw_color(uint32_t texel,const uint16_t p[16]){
    if(!texel)return 0;
    if((texel>>24)==246)return (texel&15)?bgr(p[texel&15]):0;
    int flat=1;for(int k=3;k<=11;k++)flat&=(p[k]&0x7fff)==(p[2]&0x7fff);
    if(flat)return bgr(p[2]);
    /* Character outline entries and weapon slots are runtime-controlled.
     * Keep the illustration's RGB body colours; transfer a changed outline. */
    unsigned tag=texel>>24;int character=tag==253?0:tag==254?1:tag>=247&&tag<=251?253-(int)tag:-1;
    if(character>=0&&(p[1]&0x7fff)!=canonical[character][1])return bgr(p[1]);
    return texel|0xff000000u;
}
static int load_pose(int module,int character,const char*folder,const char*file,int cell,int expected_scale,const GfxBlob*cb,const GfxTiles*t,const GfxPalette*pal){
    int replace=-1;
    for(int i=0;i<count;i++)if(poses[i].character==character&&poses[i].cell==cell){
        if(module==CT_REDRAW_DROPIN&&poses[i].module==CT_REDRAW_DROPIN)replace=i;
        else return 1;
        break;
    }
    if(count>=POSES&&replace<0)return 0;
    Pose p={.cell=cell,.character=character,.module=module,.x0=999,.y0=999};GfxObject obj[REDRAW_OBJECTS];GfxLayout layout;
    int n=gfx_cell_objects(cb,(uint32_t)cell,obj,REDRAW_OBJECTS);
    if(!n||!gfx_cell_layout(cb,(uint32_t)cell,&layout))return 0;
    p.w=layout.w;p.h=layout.h;
    if(p.w<1||p.h<1||p.w>96||p.h>96){free(layout.tiles);return 0;}
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
    char path[1024];if(folder)snprintf(path,sizeof path,"%smods/%s/%s",rt_data_dir(),folder,file);else snprintf(path,sizeof path,"%s",file);
    FILE*check=fopen(path,"rb");unsigned char head[24];int valid=check&&fread(head,1,24,check)==24&&!memcmp(head,"\211PNG\r\n\032\n",8);if(check)fclose(check);
    unsigned iw=valid?(unsigned)head[16]<<24|(unsigned)head[17]<<16|(unsigned)head[18]<<8|head[19]:0,ih=valid?(unsigned)head[20]<<24|(unsigned)head[21]<<16|(unsigned)head[22]<<8|head[23]:0;
    if(!valid||!iw||!ih||iw>768||ih>768){free(layout.tiles);return 0;}
    SDL_Surface*raw=SDL_LoadPNG(path);if(!raw){free(layout.tiles);rt_log("redraw pilot: %s: %s",path,SDL_GetError());return 0;}
    p.scale=raw->w/p.w;
    if((expected_scale&&p.scale!=expected_scale)||(folder?(p.scale!=2&&p.scale!=4&&p.scale!=8):(p.scale<1||p.scale>8))||raw->w!=p.w*p.scale||raw->h!=p.h*p.scale){SDL_DestroySurface(raw);free(layout.tiles);rt_log("redraw pilot: cell %d has wrong canvas; original retained",cell);return 0;}
    int indexed=!folder&&raw->format==SDL_PIXELFORMAT_INDEX8;
    SDL_Surface*s=SDL_ConvertSurface(raw,SDL_PIXELFORMAT_ABGR8888);
    if(s&&indexed)for(int y=0;y<s->h;y++)for(int x=0;x<s->w;x++){unsigned index=((uint8_t*)raw->pixels)[y*raw->pitch+x]&15;((uint32_t*)((uint8_t*)s->pixels+y*s->pitch))[x]=index?(0xf6000000u|index):0;}
    SDL_DestroySurface(raw);
    if(s&&p.scale==1){SDL_Surface*large=SDL_CreateSurface(s->w*2,s->h*2,SDL_PIXELFORMAT_ABGR8888);if(large)for(int y=0;y<large->h;y++)for(int x=0;x<large->w;x++)((uint32_t*)((uint8_t*)large->pixels+y*large->pitch))[x]=((uint32_t*)((uint8_t*)s->pixels+(y/2)*s->pitch))[x/2];SDL_DestroySurface(s);s=large;p.scale=2;}
    if(!s){free(layout.tiles);return 0;}
    size_t pixels=(size_t)s->w*s->h;
    uint32_t*words=calloc(pixels,4);p.preview=calloc(pixels,4);p.original=calloc((size_t)p.w*p.h,4);p.representative=calloc((size_t)p.w*p.h,4);
    p.cols=(p.w+7)/8;int rows=(p.h+7)/8;p.tile=calloc((size_t)p.cols*rows,4);
    if(!words||!p.preview||!p.original||!p.representative||!p.tile){free(words);free(p.preview);free(p.original);free(p.representative);free(p.tile);SDL_DestroySurface(s);free(layout.tiles);return 0;}
    uint8_t original[96*96]={0};gfx_draw(&layout,t,original);free(layout.tiles);
    for(int k=0;k<p.w*p.h;k++)if(original[k])p.original[k]=0xff000000u|rgb(pal->c[original[k]]);
    unsigned opaque_pixels=0;
    for(int y=0;y<s->h;y++)for(int x=0;x<s->w;x++){
        uint32_t c=((uint32_t*)((uint8_t*)s->pixels+y*s->pitch))[x];
        if((c>>24)<128)continue;opaque_pixels++;
        int edge=0;
        if(!indexed&&(c&255)+((c>>8)&255)+((c>>16)&255)<288)for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){
            int xx=x+dx,yy=y+dy;
            if(xx<0||yy<0||xx>=s->w||yy>=s->h||(((uint32_t*)((uint8_t*)s->pixels+yy*s->pitch))[xx]>>24)<128)edge=1;
        }
        size_t i=(size_t)y*s->w+x;words[i]=indexed?c:(c&0xffffffu)|((edge?(character==0?253u:character==1?254u:253u-(unsigned)character):255u)<<24);p.preview[i]=indexed?0xff000000u|rgb(pal->c[c&15]):c|0xff000000u;
        p.representative[(y/p.scale)*p.w+x/p.scale]=words[i];
    }
    if(!opaque_pixels){rt_log("redraw: %s is empty; original retained",path);free(words);free(p.preview);free(p.original);free(p.representative);free(p.tile);SDL_DestroySurface(s);return 0;}
    for(int ty=0;ty<rows;ty++)for(int tx=0;tx<p.cols;tx++){
        int dim=8*p.scale;uint32_t*px=calloc((size_t)dim*dim,4);if(!px)continue;
        for(int y=0;y<dim;y++)for(int x=0;x<dim;x++){
            int sx=tx*dim+x,sy=ty*dim+y;if(sx<s->w&&sy<s->h)px[y*dim+x]=words[(size_t)sy*s->w+sx];
        }
        p.tile[ty*p.cols+tx]=gfxmod_add_direct(p.scale,3,px);
    }
    free(words);SDL_DestroySurface(s);
    for(int i=0;i<p.cols*rows;i++)if(!p.tile[i]){free(p.preview);free(p.original);free(p.representative);free(p.tile);return 0;}
    if(replace>=0){
        Pose*old=poses+replace;free(old->preview);free(old->original);free(old->representative);
        for(int i=0;i<old->cols*((old->h+7)/8);i++)gfxmod_release_direct(old->tile[i]);
        free(old->tile);*old=p;
    }else poses[count++]=p;
    rt_log("redraw: loaded %s cell %d, %dx%d at %dx, %d nonblank original objects",characters[character],cell,p.w,p.h,p.scale,p.ref.count);return 1;
}
int ct_redraw_dropin(const char*path,const char*rel){
    int character,cell,used=0;if(sscanf(rel,"frames/Chara/Chara_%4d/%3d.png%n",&character,&cell,&used)!=2||rel[used]||character<0||character>6||cell<0)return 0;
    char expected[96];snprintf(expected,sizeof expected,"frames/Chara/Chara_%04d/%03d.png",character,cell);if(strcmp(expected,rel))return 0;
    if(strcmp(rt_rom.code,"YQUE")||!gfx_list_files())return 1;
    char name[128];snprintf(name,sizeof name,"Chara/Chara_%04d.NCGR",character);GfxBlob tb=gfx_open(gfx_find(name));snprintf(name,sizeof name,"Chara/Chara_%04d.NCER",character);GfxBlob cb=gfx_open(gfx_find(name));GfxTiles t;GfxPalette pal;snprintf(name,sizeof name,"Chara/ObjPlt_%04d.NCLR",character);
    int good=gfx_read_tiles(&tb,&t)&&!t.bpp8&&gfx_read_palette(gfx_find(name),&pal)&&pal.colors>=16&&(unsigned)cell<gfx_cell_count(&cb);
    if(good){for(int i=0;i<16;i++)canonical[character][i]=(pal.c[i].r>>3)|((pal.c[i].g>>3)<<5)|((pal.c[i].b>>3)<<10);good=load_pose(CT_REDRAW_DROPIN,character,NULL,path,cell,0,&cb,&t,&pal);}
    if(!good)rt_log("PNG override: %s rejected or unsupported object layout; earlier/original art retained",rel);
    SDL_SetAtomicInt(&gameplay[CT_REDRAW_DROPIN],1);gfx_close(&tb);gfx_close(&cb);return 1;
}
static void load_module(int module,int character,const char*folder,const char*ini,const char*prefix,const int*ids,int n){
    char path[1024];snprintf(path,sizeof path,"%smods/%s/%s",rt_data_dir(),folder,ini);
    FILE*f=fopen(path,"r");if(!f)return;
    char line[128];int enabled=0,live=1;
    while(fgets(line,sizeof line,f)){int v;char extra;
        if(sscanf(line," enabled = %d %c",&v,&extra)==1)enabled=v==1;
        else if(sscanf(line," gameplay = %d %c",&v,&extra)==1)live=v==1;
    }fclose(f);if(!enabled)return;
    char name[128];snprintf(name,sizeof name,"Chara/Chara_%04d.NCGR",character);GfxBlob tb=gfx_open(gfx_find(name));
    snprintf(name,sizeof name,"Chara/Chara_%04d.NCER",character);GfxBlob cb=gfx_open(gfx_find(name));
    GfxTiles t;GfxPalette pal;snprintf(name,sizeof name,"Chara/ObjPlt_%04d.NCLR",character);
    if(!gfx_read_tiles(&tb,&t)||t.bpp8||!gfx_read_palette(gfx_find(name),&pal)||pal.colors<16)goto done;
    for(int i=0;i<16;i++)canonical[character][i]=(pal.c[i].r>>3)|((pal.c[i].g>>3)<<5)|((pal.c[i].b>>3)<<10);
    for(int k=0;k<n&&count<POSES;k++){char file[64];snprintf(file,sizeof file,"%s-%03d.png",prefix,ids[k]);load_pose(module,character,folder,file,ids[k],0,&cb,&t,&pal);}
    SDL_SetAtomicInt(&gameplay[module],live);
    rt_log("redraw: %s module: %d loaded cells, gameplay %s",prefix,ct_redraw_module_count(module),live?"on":"off");
done:gfx_close(&tb);gfx_close(&cb);
}
/* JSON entries actively select PNGs; names remain identical to the dump. */
static void load_crono_manifest(void){
    char path[1024],error[192];snprintf(path,sizeof path,"%smods/crono-hd/mod.json",rt_data_dir());
    FILE*f=fopen(path,"rb");if(!f)return;
    if(fseek(f,0,SEEK_END)!=0){fclose(f);rt_log("redraw: cannot seek Crono manifest");return;}
    long length=ftell(f);if(length<=0||length>512*1024||fseek(f,0,SEEK_SET)!=0){fclose(f);rt_log("redraw: Crono manifest is empty or oversized");return;}
    char*json=malloc((size_t)length);RedrawManifest*m=malloc(sizeof *m);
    int okay=json&&m&&fread(json,1,(size_t)length,f)==(size_t)length;if(fclose(f))okay=0;
    if(!okay){free(json);free(m);rt_log("redraw: cannot read Crono manifest");return;}
    okay=redraw_manifest_parse(json,(size_t)length,m,error,sizeof error);free(json);
    if(!okay){rt_log("redraw: Crono manifest rejected: %s",error);free(m);return;}
    if(!m->enabled){free(m);return;}
    GfxBlob tb=gfx_open(gfx_find("Chara/Chara_0000.NCGR")),cb=gfx_open(gfx_find("Chara/Chara_0000.NCER"));GfxTiles t;GfxPalette pal;
    if(!gfx_read_tiles(&tb,&t)||t.bpp8||!gfx_read_palette(gfx_find("Chara/ObjPlt_0000.NCLR"),&pal)||pal.colors<16)goto done;
    for(int i=0;i<16;i++)canonical[0][i]=(pal.c[i].r>>3)|((pal.c[i].g>>3)<<5)|((pal.c[i].b>>3)<<10);
    for(int k=0;k<m->count&&count<POSES;k++){
        const RedrawFrameSpec*entry=m->frames+k;if(!entry->enabled)continue;
        if((unsigned)entry->cell>=gfx_cell_count(&cb)){rt_log("redraw: Crono cell %d is outside the ROM; original retained",entry->cell);continue;}
        load_pose(CT_REDRAW_CRONO,0,"crono-hd",entry->file,entry->cell,m->scale,&cb,&t,&pal);
    }
    int live=1;snprintf(path,sizeof path,"%smods/crono-hd/pack.ini",rt_data_dir());f=fopen(path,"r");
    if(f){char line[128];while(fgets(line,sizeof line,f)){int v;char extra;if(sscanf(line," gameplay = %d %c",&v,&extra)==1)live=v==1;}fclose(f);}
    SDL_SetAtomicInt(&gameplay[CT_REDRAW_CRONO],live);
    rt_log("redraw: Crono JSON registered %d/%d entries; gameplay %s; unlisted cells remain original",ct_redraw_module_count(CT_REDRAW_CRONO),m->count,live?"on":"off");
done:gfx_close(&tb);gfx_close(&cb);free(m);
}
void ct_redraw_load(void){
    if(strcmp(rt_rom.code,"YQUE")||!gfx_list_files())return;
    static const int marle[]={89,90,91,211};
    load_module(CT_REDRAW_MARLE,1,"marle-pilot","pilot.ini","marle",marle,4);
    load_crono_manifest();
    SDL_SetAtomicInt(&visible,count>0);
    rt_log("redraw: original game selects and times cells; Marle 211 remains comparison-only");
}
int ct_redraw_count(void){return count;}
int ct_redraw_pose_cell(int p){return p>=0&&p<count?poses[p].cell:-1;}
int ct_redraw_module_count(int m){if(m<0||m>=CT_REDRAW_MODULES)return 0;int n=0;for(int p=0;p<count;p++)n+=poses[p].module==m;return n;}
int ct_redraw_module_enabled(int m){return m>=0&&m<CT_REDRAW_MODULES&&SDL_GetAtomicInt(&gameplay[m]);}
void ct_redraw_module_set_enabled(int m,int on){if(m>=0&&m<CT_REDRAW_MODULES)SDL_SetAtomicInt(&gameplay[m],on!=0);}
int ct_redraw_enabled(void){return ct_redraw_module_enabled(CT_REDRAW_MARLE);}
void ct_redraw_set_enabled(int on){ct_redraw_module_set_enabled(CT_REDRAW_MARLE,on);}
void ct_redraw_set_visible(int on){SDL_SetAtomicInt(&visible,on&&count);}
static int palette_ok(int character,const uint16_t*p){
    int same=1,flat=1;for(int k=2;k<=11;k++){same&=(p[k]&0x7fff)==canonical[character][k];flat&=(p[k]&0x7fff)==(p[2]&0x7fff);}
    return same||flat;
}
int ct_redraw_prepare(int e,const RedrawObject live[128],const uint16_t palette[256]){
    if(e<0||e>=2)return 0;memset(bindings[e],0,sizeof bindings[e]);if(!SDL_GetAtomicInt(&visible))return 0;
    int groups=0;
    for(int p=0;p<count;p++)for(int a=0;a<128;a++){
        Pose*f=poses+p;
        if(!ct_redraw_module_enabled(f->module)||bindings[e][a].action||!live[a].valid||live[a].palette<0||live[a].palette>15||!palette_ok(f->character,palette+live[a].palette*16))continue;
        int slots[REDRAW_OBJECTS];
        if(f->module==CT_REDRAW_MARLE&&f->character==1&&f->cell==211)continue; /* Reported waiting/hit mismatch: comparison only until redrawn. */
        if(!redraw_match(&f->ref,live,a,slots))continue;
        int used=0,draw=slots[0];for(int k=0;k<f->ref.count;k++){used|=bindings[e][slots[k]].action;if(slots[k]<draw)draw=slots[k];}
        if(used)continue;
        for(int k=0;k<f->ref.count;k++)bindings[e][slots[k]].action=1;
        bindings[e][draw]=(RedrawBinding){2,p,(live[a].x-f->ref.object[0].x+f->x0)&511,(live[a].y-f->ref.object[0].y+f->y0)&255,f->w,f->h,live[slots[f->top]].above};groups++;
        if(!f->seen){f->seen=1;rt_log("redraw: matched authored %s cell %d in engine %d",characters[f->character],f->cell,e);}
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
const char*ct_redraw_viewer_title(void){static char title[64];snprintf(title,sizeof title,"%s cell %03d - art pilot",count?characters[poses[selected].character]:"Character",count?poses[selected].cell:0);return title;}
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
