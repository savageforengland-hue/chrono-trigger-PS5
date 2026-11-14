/* Proportional presentation geometry for screens without additional scene data. */
#pragma once
typedef struct CtPresentRect { float x, y, w, h; } CtPresentRect;
CtPresentRect ctps5_present_rect(int output_w, int output_h, int content_w, int content_h,
                                int mode, int integer_scale, int wide_enabled);
