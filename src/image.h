#ifndef __SRC_IMAGE_H___
#define __SRC_IMAGE_H___

#include "types.h"

image_t image_create(u32 width, u32 height, u32* pixel_buffer);

void image_set_pixel(image_t *img, u32 x, u32 y, u32 color);

#endif // __SRC_IMAGE_H___
