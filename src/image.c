#include "image.h"
#include <stdio.h>

const u32 xy_to_index(const u32 x, const u32 y, const u32 width)
{
	return width * y + x;
}

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
	img->pixel_buffer[xy_to_index(x, y, img->width)] = color;
}

void image_draw_rectangle_color(image_t *img, u32 x, u32 y, u32 w, u32 h, u32 color)
{
	u32 right = x+w;
	u32 bottom = y+h;

	if (right >= img->width) right = img->width-1;
	if (bottom >= img->height) bottom = img->height-1;

	for (u32 yy = y; yy <= bottom; yy++) {
		for (u32 xx = x; xx <= right ; xx++) {
			img->pixel_buffer[xy_to_index(xx, yy, img->width)] = color;
		}
	}
}
