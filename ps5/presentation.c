/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "presentation.h"
#include <math.h>

CtPresentRect ctps5_present_rect(int ow, int oh, int cw, int ch,
                                int mode, int integer_scale, int wide_enabled) {
    if (ow <= 0 || oh <= 0 || cw <= 0 || ch <= 0) return (CtPresentRect){0};
    const float sx = (float)ow / cw, sy = (float)oh / ch;
    const int narrow = wide_enabled && (float)cw / ch < (float)ow / oh - 0.02f;
    if (narrow && mode == 3) return (CtPresentRect){0, 0, (float)ow, (float)oh};
    float scale = narrow && mode == 2 ? fmaxf(sx, sy) : fminf(sx, sy);
    if (integer_scale && scale >= 1 && !(narrow && mode >= 2)) scale = floorf(scale);
    float w = cw * scale, h = ch * scale;
    return (CtPresentRect){floorf((ow - w) / 2), floorf((oh - h) / 2), w, h};
}
