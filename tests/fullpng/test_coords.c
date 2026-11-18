/* Pure field-map ring-origin regression, no SDL or ROM. */
#include <assert.h>
#include <stdio.h>
#include "fullpng_coords.h"

int main(void) {
    /* Native camera at tile 100, origin slot 0, fine scroll 3. */
    assert(fullpng_ring_origin(100, 0, 3, 64) == 803);
    /* Rightward scrolling across a ring slot boundary. */
    assert(fullpng_ring_origin(100, 63, 0, 64) == 808);
    /* Leftward scroll wraps into the negative ring displacement. */
    assert(fullpng_ring_origin(100, 0, 62u * 8u + 5u, 64) == 789);
    /* Vertical 32-row ring uses a different wrap interval. */
    assert(fullpng_ring_origin(40, 31, 1, 32) == 329);
    assert(fullpng_ring_origin(40, 0, 31u * 8u + 7u, 32) == 319);
    /* Fine scroll must not produce a discontinuity between tiles. */
    for (unsigned x = 0; x < 7; ++x) {
        assert(fullpng_ring_origin(8, 5, (5u * 8u) + x + 1u, 64) ==
               fullpng_ring_origin(8, 5, (5u * 8u) + x, 64) + 1);
    }
    puts("PASS: full-PNG camera coordinates, fine scroll and ring wrap");
    return 0;
}
