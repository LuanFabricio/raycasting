#ifndef __SRC_UTILS_H__
#define __SRC_UTILS_H__

#include <emmintrin.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <immintrin.h>
#include <xmmintrin.h>

#include "types.h"
#include "defines.h"

#define MIN(x, y) (x) > (y) ? (y) : (x)
#define MAX(x, y) (x) < (y) ? (y) : (x)

#define CAST_TYPE(target, var) *(target*)&(var)

#define DOUBLE_EQUAL(d1, d2) (fabs((d1) - (d2)) > DBL_EPSILON)

u32 xy_to_index(const u32 x, const u32 y, const u32 width);
vec2u32_t index_to_xy(u32 index, u32 width);
f32 lerp(f32 start, f32 end, f32 amount);
u8 u32_to_color_channel(u32 color, color_channel_e channel);
u32 color_channel_to_u32(u8 color, color_channel_e channel);
u32 color_apply_shadow(u32 color_u32, f32 shadow);
u32 blend_colors(const u32 src, const u32 dest);
f32 update_player_angle(const portal_t *portal1, const portal_t *portal2);
f32 calc_perp_dist(const vec2f32_t *hit, const vec2f32_t *pos, const vec2f32_t *pos_ray);

#endif // __SRC_UTILS_H__
