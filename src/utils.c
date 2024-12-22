#include "utils.h"
#include "types.h"
#include "vec2f32.h"
#include <math.h>
#include <stdio.h>

u32 xy_to_index(const u32 x, const u32 y, const u32 width)
{
		return width * y + x;
}

vec2u32_t index_to_xy(u32 index, u32 width) {
	vec2u32_t r = {0};

	r.x = index % width;
	r.y = (index - r.x) / width;

	return r;
}

f32 lerp(f32 start, f32 end, f32 amount)
{
	return start + amount * (end-start);
}

u8 u32_to_color_channel(u32 color, color_channel_e channel)
{
	return ((0xff << (8 * channel)) & color) >> (8 * channel);
}

u32 color_channel_to_u32(u8 color, color_channel_e channel)
{
	return color << (8 * channel);
}

u32 color_apply_shadow(u32 color_u32, f32 shadow)
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

u32 blend_colors(const u32 src, const u32 dest)
{
	const u8 src_alpha_u8 = u32_to_color_channel(src, COLOR_CHANNEL_ALPHA);
	if (src_alpha_u8 == 0xff) return src;
	if (src_alpha_u8 == 0x00) return dest;

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
	__m128 vec_dest_rgb = _mm_set_ps(r_dest, g_dest, b_dest, 0);
	vec_dest_rgb = _mm_mul_ps(vec_dest_alpha, vec_dest_rgb);

	__m128 vec_blend_color = _mm_add_ps(vec_dest_rgb, vec_src_rgb);
	f32 *blend_color = (f32*)&vec_blend_color;

	const u32 r = color_channel_to_u32((u32)blend_color[3], COLOR_CHANNEL_RED);
	const u32 g = color_channel_to_u32((u32)blend_color[2], COLOR_CHANNEL_GREEN);
	const u32 b = color_channel_to_u32((u32)blend_color[1], COLOR_CHANNEL_BLUE);

	const u32 a = color_channel_to_u32(0xff, COLOR_CHANNEL_ALPHA);
	return r | g | b | a;
}

f32 update_player_angle(const portal_t *portal)
{
	const block_face_e v1_face = portal->block_src->portal_face;
	const block_face_e v2_face = portal->block_dest->portal_face;
	const f32 match_table[BLOCK_FACE_LEFT+1][BLOCK_FACE_LEFT+1] = {
		// UP 	    | RIGHT	| DOWN	    | LEFT	|
		{ PI	    , 3 * PI / 2, 0	    , PI / 2	}, // BLOCK_FACE_UP
		{ PI / 2    , PI	, 3 * PI / 2, 0 	}, // BLOCK_FACE_RIGHT
		{ 0	    , PI / 2	, PI 	    , 3 * PI / 2}, // BLOCK_FACE_DOWN
		{ 3 * PI / 2, 0		, PI / 2    , PI	}, // BLOCK_FACE_LEFT
	};

	return match_table[v1_face][v2_face];
}
