/* Pure alpha compositing regression for full PNG subpixels. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "fullpng_blend.h"

int main(void) {
    const uint32_t blue = 0xFFFF0000u; /* ABGR8888 */
    const uint32_t red = 0xFF0000FFu;
    assert(fullpng_blend_abgr(red, blue, 0) == blue);
    assert(fullpng_blend_abgr(red, blue, 255) == red);
    assert(fullpng_blend_abgr(red, blue, 256) == red);
    assert(fullpng_blend_abgr(red, blue, 128) == 0xFF7F0080u);
    /* Black and white keep channels clipped and always opaque. */
    assert(fullpng_blend_abgr(0xFFFFFFFFu, 0xFF000000u, 128) == 0xFF808080u);
    puts("PASS: full-PNG transparent, opaque and semi-transparent composition");
    return 0;
}
