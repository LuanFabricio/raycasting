#ifndef __SRC_UTILS_H__
#define __SRC_UTILS_H__

#include "types.h"
#include <math.h>
#include <float.h>
#include <stdio.h>

typedef enum {
	COLOR_CHANNEL_ALPHA = 3,
	COLOR_CHANNEL_BLUE = 2,
	COLOR_CHANNEL_GREEN = 1,
	COLOR_CHANNEL_RED = 0,
} color_channel_e;

#define MIN(x, y) (x) > (y) ? (y) : (x)
#define MAX(x, y) (x) < (y) ? (y) : (x)

#define CAST_TYPE(target, var) *(target*)&var

#define DOUBLE_EQUAL(d1, d2) (fabs((d1) - (d2)) > DBL_EPSILON)

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

static u32 color_channel_to_u32(u8 color, color_channel_e channel)
{
	return color << (8 * channel);
}

static u32 color_apply_shadow(u32 color_u32, f32 shadow)
{
	const u32 r = color_channel_to_u32(
			u32_to_color_channel(color_u32, COLOR_CHANNEL_RED) * shadow,
			COLOR_CHANNEL_RED);
	const u32 g = color_channel_to_u32(
			u32_to_color_channel(color_u32, COLOR_CHANNEL_GREEN) * shadow,
			COLOR_CHANNEL_GREEN);
	const u32 b = color_channel_to_u32(
			u32_to_color_channel(color_u32, COLOR_CHANNEL_BLUE) * shadow,
			COLOR_CHANNEL_BLUE);
	const u32 a = color_channel_to_u32(
			u32_to_color_channel(color_u32, COLOR_CHANNEL_ALPHA),
			COLOR_CHANNEL_ALPHA);

	return r | g | b | a;
}

static u32 blend_colors(const u32 src, const u32 dest)
{
	const f32 src_alpha = (f32)u32_to_color_channel(src, COLOR_CHANNEL_ALPHA) / 0xff;
	const f32 dest_alpha = 1.0f - src_alpha;
	// printf("Src alpha: %.08f(0x%08x|0x%08x)\n", src_alpha, src, u32_to_color_channel(src, COLOR_CHANNEL_ALPHA));

	const u8 r_src = u32_to_color_channel(src, COLOR_CHANNEL_RED);
	const u8 g_src = u32_to_color_channel(src, COLOR_CHANNEL_GREEN);
	const u8 b_src = u32_to_color_channel(src, COLOR_CHANNEL_BLUE);

	const u8 r_dest = u32_to_color_channel(dest, COLOR_CHANNEL_RED);
	const u8 g_dest = u32_to_color_channel(dest, COLOR_CHANNEL_GREEN);
	const u8 b_dest = u32_to_color_channel(dest, COLOR_CHANNEL_BLUE);

	const u32 r = color_channel_to_u32(r_src * src_alpha + r_dest * dest_alpha, COLOR_CHANNEL_RED);
	const u32 g = color_channel_to_u32(g_src * src_alpha + g_dest * dest_alpha, COLOR_CHANNEL_GREEN);
	const u32 b = color_channel_to_u32(b_src * src_alpha + b_dest * dest_alpha, COLOR_CHANNEL_BLUE);

	const u32 a = color_channel_to_u32(0xff, COLOR_CHANNEL_ALPHA);
	return r | g | b | a;
}


#endif // __SRC_UTILS_H__
