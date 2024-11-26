#include <math.h>

#include "types.h"
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

u32 render_get_texture_x(const vec2f32_t *hit_point, const u8 block_face)
{
	vec2f32_t grid_point = {0};
	vec2f32_floor(hit_point, &grid_point);

	vec2f32_t tex_point = {0};
	vec2f32_sub(hit_point, &grid_point, &tex_point);

	f32 u = 0;
	switch (block_face) {
		case 0:
			u = 1 - tex_point.x;
			break;
		case 1:
			u = 1 - tex_point.y;
			break;
		case 2:
			u = tex_point.x;
			break;
		case 3:
			u = tex_point.y;
			break;
		default:
			break;
	}

	return floorf(u * TEXTURE_SIZE);
}

u32 render_get_texture_y(const u32 screen_y, const u32 render_y, const f32 text_height_prop)
{
	return floorf((screen_y - render_y) * text_height_prop);
}

u32 render_get_texture_color_index(const u32 tex_x, const u32 tex_y, const u32 tex_height)
{
	return tex_y * tex_height + tex_x;
}
