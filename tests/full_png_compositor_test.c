#include "../runtime/full_png_compositor.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    assert(ct_png_over(0,0x123456ffu)==0x123456ffu);
    assert(ct_png_over(0x112233ffu,0)==0x112233ffu);
    uint32_t bg[]={0xff0000ffu,0xff0000ffu,0xff0000ffu,0xff0000ffu};
    uint32_t fg[]={0,0,0x0000ffffu,0};
    uint32_t actor[]={0,0x00ff00ffu,0,0};
    uint32_t out[4]={0};
    CtPngLayer b={bg,2,2,2,0,0,1},f={fg,2,2,2,0,0,1};
    ct_png_compose(out,2,2,2,&b,&f,actor,2,0,0);
    assert(out[0]==0xff0000ffu);
    assert(out[1]==0x00ff00ffu);
    assert(out[2]==0x0000ffffu);
    assert(out[3]==0xff0000ffu);
    ct_png_compose(out,1,1,2,&b,&f,actor,2,100,100);
    assert(out[0]==0);
    puts("full PNG compositor tests passed");
    return 0;
}
