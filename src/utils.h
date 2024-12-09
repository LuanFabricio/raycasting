#ifndef __SRC_UTILS_H__
#define __SRC_UTILS_H__

#include <emmintrin.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <immintrin.h>
#include <xmmintrin.h>

#include "types.h"

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
	const u8 src_alpha_u8 = u32_to_color_channel(src, COLOR_CHANNEL_ALPHA);
	if (src_alpha_u8 == 0xff) return src;

	const f32 src_alpha = (f32)src_alpha_u8 / 0xff;
	const f32 dest_alpha = 1.0f - src_alpha;

	const u8 r_src = u32_to_color_channel(src, COLOR_CHANNEL_RED);
	const u8 g_src = u32_to_color_channel(src, COLOR_CHANNEL_GREEN);
	const u8 b_src = u32_to_color_channel(src, COLOR_CHANNEL_BLUE);

	__m128 vec_src_alpha = _mm_set_ps1(src_alpha);
	__m128 vec_src_rgb = _mm_set_ps(r_src, g_src, b_src, 0);
	vec_src_rgb = _mm_mul_ps(vec_src_alpha, vec_src_rgb);

	const u8 r_dest = u32_to_color_channel(dest, COLOR_CHANNEL_RED);
	const u8 g_dest = u32_to_color_channel(dest, COLOR_CHANNEL_GREEN);
	const u8 b_dest = u32_to_color_channel(dest, COLOR_CHANNEL_BLUE);

	__m128 vec_dest_alpha = _mm_set_ps1(dest_alpha);
	__m128 vec_dest_rgb = _mm_set_ps(r_dest, g_dest, g_dest, 0);
	vec_dest_rgb = _mm_mul_ps(vec_dest_alpha, vec_dest_rgb);

	__m128 vec_blend_color = _mm_add_ps(vec_dest_rgb, vec_src_rgb);
	f32 *blend_color = (f32*)&vec_blend_color;

	const u32 r = color_channel_to_u32((u32)blend_color[3], COLOR_CHANNEL_RED);
	const u32 g = color_channel_to_u32((u32)blend_color[2], COLOR_CHANNEL_GREEN);
	const u32 b = color_channel_to_u32((u32)blend_color[1], COLOR_CHANNEL_BLUE);

	const u32 a = color_channel_to_u32(0xff, COLOR_CHANNEL_ALPHA);
	return r | g | b | a;
}

#endif // __SRC_UTILS_H__
