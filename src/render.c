#include <math.h>

#include "collision.h"
#include "types.h"
#include "vec2f32.h"
#include "utils.h"
#include "image.h"

#include "defines.h"

#include "render.h"

void get_fov_plane(const vec2f32_t pos, const f32 angle, const f32 scale, vec2f32_t out[2])
{
	const f32 half_fov = FOV * 0.5f;
	out[0] = vec2f32_from_angle(angle - half_fov);
	vec2f32_scale(&out[0], scale, &out[0]);
	vec2f32_add(&pos, &out[0], &out[0]);

	out[1] = vec2f32_from_angle(angle + half_fov);
	vec2f32_scale(&out[1], scale, &out[1]);
	vec2f32_add(&pos, &out[1], &out[1]);
}

u32 render_get_texture_x(const vec2f32_t *hit_point, const block_face_e block_face)
{
	vec2f32_t grid_point = {0};
	vec2f32_floor(hit_point, &grid_point);

	vec2f32_t tex_point = {0};
	vec2f32_sub(hit_point, &grid_point, &tex_point);

	f32 u = 0;
	switch (block_face) {
		case BLOCK_FACE_UP:
			u = 1 - tex_point.x;
			break;
		case BLOCK_FACE_RIGHT:
			u = 1 - tex_point.y;
			break;
		case BLOCK_FACE_DOWN:
			u = tex_point.x;
			break;
		case BLOCK_FACE_LEFT:
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

void render_block_color_on_image(
	const u32 x, const f32 y,
	const u32 width, const u32 height,
	const u32 color, const f32 shadow,
	image_t *image
) {
	u32 color_u32 = color_apply_shadow(color, shadow);
	const u32 image_y = MAX(0, y);
	image_draw_rectangle_color(image, x, image_y, width, height, color_u32);
}

void render_block_texture_on_image(
	const render_texture_t *tex_data,
	const u32 screen_x,
	const u32 screen_height,
	const f32 shadow,
	image_t *image
) {
	const f32 text_height_prop = tex_data->size.y / tex_data->strip.y;

	const i32 y_start = MAX(0, tex_data->coords.y);
	const i32 y_end = MIN(screen_height, tex_data->coords.y + tex_data->strip.y);

	for (i32 y = y_start; y < y_end; y++) {
		const u32 tex_y = render_get_texture_y(y, tex_data->coords.y, text_height_prop);
		const u32 tex_point = render_get_texture_color_index(
				tex_data->coords.x, tex_y, tex_data->size.y);

		const u32 color_u32 = color_apply_shadow(tex_data->pixels[tex_point], shadow);
		image_draw_rectangle_color(image, screen_x, y, tex_data->strip.x, 1, color_u32);
	}
}

void render_portal(
	const scene_t *scene,
	const collision_block_t collision_block,
	const u32 x, const f32 y,
	const u32 strip_width, const u32 strip_height,
	const u32 screen_height,
	const f32 shadow,
	const portal_t *portal_ptr,
	image_t *image
)
{
	const bool block_face_match_portal = portal_ptr->block_src == collision_block.block_ptr
		&& portal_ptr->face == collision_block.face;
	if (block_face_match_portal) {
		render_texture_t tex_data = {
			.coords = { render_get_texture_x(&collision_block.hit, collision_block.face), y },
			.strip = { strip_width, strip_height },
			.size = { TEXTURE_SIZE, TEXTURE_SIZE },
			.pixels = portal_ptr->pixels,
		};
		render_block_texture_on_image(
			&tex_data,
			x*strip_width, screen_height,
			shadow, image);
	}
}

void render_scene_on_image(
	const scene_t *scene,
	const u32 screen_width, const u32 screen_height,
	image_t* image
)
{
	const u32 strip_width = screen_width / RENDER_WIDTH;
	// const u32 strip_height = 1 + screen_height / RENDER_HEIGHT;

	vec2f32_t fov_plane[2] = {0};
	get_fov_plane(scene->player.position, scene->player.angle, FAR_DISTANCE, fov_plane);

	vec2f32_t player_ray = vec2f32_from_angle(scene->player.angle);
	vec2f32_norm(&player_ray, &player_ray);

	for (u32 x = 0; x < RENDER_WIDTH; x++) {
		vec2f32_t ray = {0};
		const f32 amount = (f32)x / (float)RENDER_WIDTH;
		vec2f32_lerp(&fov_plane[0], &fov_plane[1], amount, &ray);

		collision_block_t collision_block = collision_block_empty();
		const bool res = collision_hit_a_block(
				scene, scene->player.position, ray, &collision_block);

		if (!res) continue;

		vec2f32_t v = {0};
		vec2f32_sub(&collision_block.hit, &scene->player.position, &v);
		const f32 perp_wall_dist = vec2f32_dot(&v, &player_ray);
		const f32 strip_height = (f32)screen_height / perp_wall_dist;
		const f32 y = (screen_height - strip_height) * 0.5f;

		const f32 shadow = MIN(1.0f/perp_wall_dist*4.0f, 1.0f);

		switch (collision_block.block_ptr->block_type) {
			case BLOCK_COLOR: {
				const u32 color = *(u32*)collision_block.block_ptr->data;
				render_block_color_on_image(
					x*strip_width, y,
					strip_width, strip_height,
					color, shadow,
					image);
			} break;

			case BLOCK_BRICKS: {
				const u32 src_x = render_get_texture_x(&collision_block.hit, collision_block.face);
				const render_texture_t tex_data = {
					.pixels = collision_block.block_ptr->data,
					.coords = { src_x, y },
					.strip = { strip_width, strip_height },
					.size = { TEXTURE_SIZE, TEXTURE_SIZE },
				};
				render_block_texture_on_image(
					&tex_data,
					x*strip_width, screen_height,
					shadow, image);

			} break;

			default:
				continue;
				break;
		}

		render_portal(scene, collision_block, x, y, strip_width, strip_height, screen_height, shadow, &scene->portal1, image);
		render_portal(scene, collision_block, x, y, strip_width, strip_height, screen_height, shadow, &scene->portal2, image);
	}
}
