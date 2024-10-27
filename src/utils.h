#ifndef __SRC_UTILS_H__
#define __SRC_UTILS_H__

#include "types.h"

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

#endif // __SRC_UTILS_H__
