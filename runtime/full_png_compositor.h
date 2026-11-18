/* Full-PNG layer compositing primitive. Independent of SDL/PS5.
 * Pixels are straight-alpha RGBA8 in 0xRRGGBBAA order.
 * Scene coordinates and camera origin are in native DS pixels; scale is
 * an integer number of PNG pixels per native pixel.
 * This is a staging primitive, NOT wired into the live renderer yet.
 */
#ifndef CTPS5_FULL_PNG_COMPOSITOR_H
#define CTPS5_FULL_PNG_COMPOSITOR_H
#include <stdint.h>
#include <stddef.h>
typedef struct CtPngLayer {
    const uint32_t *rgba;
    int width, height, stride;
    int origin_x, origin_y; /* scene-space origin, native pixels */
    int scale; /* PNG pixels per scene pixel */
} CtPngLayer;
static inline uint32_t ct_png_over(uint32_t below, uint32_t above) {
    unsigned a = above & 255u, b = below & 255u;
    unsigned out_a = a + (b * (255u-a) + 127u)/255u;
    if (!out_a) return 0;
    unsigned result = 0;
    for (unsigned shift=8; shift<=24; shift+=8) {
        unsigned src=(above>>shift)&255u, dst=(below>>shift)&255u;
        unsigned premul=src*a + (dst*b*(255u-a)+127u)/255u;
        unsigned v=(premul+out_a/2)/out_a;
        if(v>255u) v=255u;
        result |= v<<shift;
    }
    return result | out_a;
}
static inline uint32_t ct_png_sample(const CtPngLayer *layer, int scene_x, int scene_y) {
    if (!layer || !layer->rgba || layer->scale<=0 || layer->width<=0 ||
        layer->height<=0 || layer->stride<layer->width) return 0;
    int64_t x=((int64_t)scene_x-layer->origin_x)*layer->scale;
    int64_t y=((int64_t)scene_y-layer->origin_y)*layer->scale;
    if(x<0 || y<0 || x>=layer->width || y>=layer->height) return 0;
    return layer->rgba[(size_t)y*(size_t)layer->stride+(size_t)x];
}
static inline void ct_png_compose(uint32_t *out, int width, int height, int stride,
    const CtPngLayer *background, const CtPngLayer *foreground,
    const uint32_t *actors, int actor_stride, int camera_x, int camera_y) {
    if(!out || width<=0 || height<=0 || stride<width) return;
    for(int y=0;y<height;y++) for(int x=0;x<width;x++) {
        uint32_t bg=ct_png_sample(background,camera_x+x,camera_y+y);
        uint32_t actor=(actors && actor_stride>=width)?actors[(size_t)y*actor_stride+x]:0;
        uint32_t fg=ct_png_sample(foreground,camera_x+x,camera_y+y);
        out[(size_t)y*stride+x]=ct_png_over(ct_png_over(bg,actor),fg);
    }
}
#endif
