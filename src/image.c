#include "image.h"

image_t image_create(u32 width, u32 height, u32* pixel_buffer)
{
	return (image_t) {
		.width = width,
		.height = height,
		.pixel_buffer = pixel_buffer,
	};
}

void image_set_pixel(image_t *img, u32 x, u32 y, u32 color)
{
	const u32 index = img->width * y + x;
	img->pixel_buffer[index] = color;
}
