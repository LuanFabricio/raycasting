#ifndef __SRC_UTILS_H__
#define __SRC_UTILS_H__

#include "types.h"
#include <math.h>
#include <float.h>

typedef enum {
	COLOR_CHANNEL_ALPHA = 3,
	COLOR_CHANNEL_RED = 2,
	COLOR_CHANNEL_GREEN = 1,
	COLOR_CHANNEL_BLUE = 0,
} color_channel_e;

#define MIN(x, y) (x) > (y) ? (y) : (x)
#define MAX(x, y) (x) < (y) ? (y) : (x)

#define CAST_TYPE(target, var) *(target*)&var

#define DOUBLE_EQUAL(d1, d2) (fabs((d1) - (d2)) > DBL_EPSILON)

static u32 blend_colors(const u32 src, const u32 target)
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

static u32 xy_to_index(const u32 x, const u32 y, const u32 width)
{
	return width * y + x;
}

static vec2u32_t index_to_xy(u32 index, u32 width) {
	vec2u32_t r = {0};

	r.x = index % width;
	r.y = (index - r.x) / width;

	return r;
}

static f32 lerp(f32 start, f32 end, f32 amount)
{
	return start + amount * (end-start);
}

static u8 u32_to_color_channel(u32 color, color_channel_e channel)
{
	return ((0xff << (8 * channel)) & color) >> (8 * channel);
}

#endif // __SRC_UTILS_H__
