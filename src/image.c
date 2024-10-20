#include "image.h"

const u32 blend_colors(const u32 src, const u32 target)
{
	const f32 src_alpha = (f32)(src & 0xff) / 0xff;
	const f32 src_alpha_c = 1.0f - src_alpha;

	// TODO: Review this
	const u32 final_color = (u32)((target & (0xff << 8 * 3)) * src_alpha_c + (src & (0xff << 8 * 3)) * src_alpha) // Red
		| (u32)((target & (0xff << 8 * 2)) * src_alpha_c + (src & (0xff << 8 * 2)) * src_alpha) // Green
		| (u32)((target & (0xff << 8 * 1)) * src_alpha_c + (src & (0xff << 8 * 1)) * src_alpha) // Blue
		| 0xff; // Alpha
	return final_color;
}

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

void image_append_image(image_t* target, const image_t* src, vec2u32_t position)
{
	u32 max_x = src->width-1;
	if (position.x + max_x >= target->width) max_x = target->width - position.x - 1;

	u32 max_y = src->height-1;
	if (position.y + max_y >= target->height) max_y = target->height - position.y - 1;

	for (u32 x = 0; x <= max_x; x++) {
		for (u32 y = 0; y <= max_y; y++) {
			const u32 target_index = xy_to_index(
					position.x+x, position.y+y, target->width);
			const u32 target_color = target->pixel_buffer[target_index];

			const u32 src_index = xy_to_index(x, y, src->width);
			const u32 src_color = src->pixel_buffer[src_index];

		   	target->pixel_buffer[target_index] = blend_colors(src_color, target_color);

		}
	}
}
