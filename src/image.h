#ifndef __SRC_IMAGE_H___
#define __SRC_IMAGE_H___

#include "types.h"

image_t image_create(u32 width, u32 height, u32* pixel_buffer);

void image_clear(image_t *img, u32 color);

void image_set_pixel(image_t *img, u32 x, u32 y, u32 color);

void image_draw_rectangle_color(image_t *img, u32 x, u32 y, u32 w, u32 h, u32 color);

void image_append_image(image_t* target, const image_t* src, vec2u32_t position);

#endif // __SRC_IMAGE_H___
