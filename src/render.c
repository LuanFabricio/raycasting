#include <math.h>

#include "types.h"
#include "utils.h"
#include "vec2f32.h"

#include "defines.h"

#include "render.h"

void get_fov_plane(const vec2f32_t pos, const f32 angle, const f32 scale, vec2f32_t out[2])
{
	vec2f32_t plane_middle = vec2f32_from_angle(BASE_ROTATION + angle);
	vec2f32_scale(&plane_middle, scale, &plane_middle);
	vec2f32_add(&plane_middle , &CAST_TYPE(vec2f32_t, pos), &plane_middle);

	f32 left = tan(FOV * 0.5) * scale;

	vec2f32_sub(&plane_middle, &CAST_TYPE(vec2f32_t, pos), &out[0]);
	vec2f32_rot90(&out[0], &out[0]);
	vec2f32_norm(&out[0], &out[0]);
	vec2f32_scale(&out[0], left, &out[0]);
	vec2f32_add(&plane_middle, &out[0], &out[0]);

	vec2f32_sub(&plane_middle, &CAST_TYPE(vec2f32_t, pos), &out[1]);
	vec2f32_rot90(&out[1], &out[1]);
	vec2f32_norm(&out[1], &out[1]);
	vec2f32_scale(&out[1], left, &out[1]);
	vec2f32_sub(&plane_middle, &out[1], &out[1]);
}
