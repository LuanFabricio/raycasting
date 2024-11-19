#include <math.h>

#include "types.h"
#include "utils.h"
#include "vec2f32.h"

#include "defines.h"

#include "render.h"

void get_fov_plane(const vec2f32_t pos, const f32 angle, const f32 scale, vec2f32_t out[2])
{
	const f32 half_fov = FOV * 0.5f;
	f32 left = scale / cos(half_fov);
	out[0] = vec2f32_from_angle(angle - half_fov);
	vec2f32_scale(&out[0], scale, &out[0]);
	vec2f32_add(&pos, &out[0], &out[0]);

	out[1] = vec2f32_from_angle(angle + half_fov);
	vec2f32_scale(&out[1], scale, &out[1]);
	vec2f32_add(&pos, &out[1], &out[1]);
}
