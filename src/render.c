#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "collision.h"
#include "scene.h"
#include "spritesheet.h"
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

u32 render_get_texture_x(const vec2f32_t *hit_point, const block_face_e block_face, const u32 width)
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

	return floorf(u * width);
}

u32 render_get_texture_y(const u32 screen_y, const u32 render_y, const f32 text_height_prop)
{
	return floorf((screen_y - render_y) * text_height_prop);
}

u32 render_get_texture_color_index(const u32 tex_x, const u32 tex_y, const u32 tex_width)
{
	return tex_y * tex_width + tex_x;
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
				tex_data->coords.x, tex_y, tex_data->size.x);

		const u32 color_u32 = color_apply_shadow(tex_data->pixels[tex_point], shadow);
		image_draw_rectangle_color(image, screen_x, y, tex_data->strip.x, 1, color_u32);
	}
}

void render_block_portal(
	const collision_block_t *cb,
	const u32 x, const f32 y,
	const u32 strip_width, const u32 strip_height,
	const u32 screen_height,
	const f32 shadow,
	const portal_t *portal_ptr,
	const image_t* portal_img,
	image_t *image
)
{
	const bool block_face_match_portal = portal_ptr->block_src == cb->block_ptr
		&& portal_ptr->face == cb->face;
	// TODO: Check the ray hit a portal pixel too
	// vec2f32_t portal_center = scene_offset_to_center_block_face(
	// 		&CAST_TYPE(vec2f32_t, cb->position), cb->face, 0.5f);
	// portal_center.x += cb->position.x;
	// portal_center.y += cb->position.y;
	// f32 prop_width = (f32)(portal_img->width) / TEXTURE_SIZE;
	// const bool hit_portal = cb->hit.x > (portal_center.x - prop_width / 2)
	// 	&& cb->hit.x < (portal_center.x + prop_width / 2);

	if (block_face_match_portal) {
		render_texture_t tex_data = {
			.coords = {
				render_get_texture_x(&cb->hit, cb->face, portal_img->width),
				y,
			},
			.strip = { strip_width, strip_height },
			.size = { portal_img->width, portal_img->height },
			.pixels = portal_img->pixel_buffer,
		};
		render_block_texture_on_image(
			&tex_data,
			x*strip_width, screen_height,
			shadow, image);
	}
}

void render_blocks(
	const u32 x, const u32 strip_width, const u32 screen_height,
	const scene_t *scene, const vec2f32_t player_ray,
	const collision_block_t *cb,
	image_t *image
)
{
	const f32 perp_wall_dist = calc_perp_dist(&cb->hit, &scene->player.position, &player_ray);
	const f32 strip_height = (f32)screen_height / perp_wall_dist;
	const f32 y = (screen_height - strip_height) * 0.5f;

	const f32 shadow = MIN(1.0f/perp_wall_dist*4.0f, 1.0f);

	switch (cb->block_ptr->block_type) {
		case BLOCK_COLOR: {
			const u32 color = *(u32*)cb->block_ptr->data;
			render_block_color_on_image(
				x*strip_width, y,
				strip_width, strip_height,
				color, shadow,
				image);
		} break;

		case BLOCK_BRICKS: {
			const u32 src_x = render_get_texture_x(&cb->hit, cb->face, TEXTURE_SIZE);
			const render_texture_t tex_data = {
				.pixels = cb->block_ptr->data,
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
			break;
	}

	render_block_portal(cb, x, y, strip_width, strip_height, screen_height, shadow, &scene->portal1, &scene->tex_map.portal1, image);
	render_block_portal(cb, x, y, strip_width, strip_height, screen_height, shadow, &scene->portal2, &scene->tex_map.portal2, image);
}

void render_entity(
	const u32 x, const u32 strip_width, const u32 screen_height,
	const scene_t *scene,
	const vec2f32_t player_ray,
	const collision_entity_t *ce,
	image_t* image
)
{
	const f32 perp_wall_dist = calc_perp_dist(&ce->entity_ptr->position, &scene->player.position, &player_ray);
	const f32 strip_height = (f32)screen_height / perp_wall_dist;
	const f32 y = (screen_height - strip_height) * 0.5f;

	const f32 shadow = MIN(1.0f/perp_wall_dist*4.0f, 1.0f);

	// BUG: This makes the sprites wider when the player is on the left or the right of the map
	const block_face_e face = (scene->player.angle < PI) ? BLOCK_FACE_DOWN : BLOCK_FACE_UP;
	vec2f32_t plane_hit = {0};
	vec2f32_rot(&ce->hit, PI, &plane_hit);

	// TODO: Test spritesheet usage
	u32 *buffer = malloc(sizeof(u32) * scene->tex_map.debug_spritesheet.sprite_size.x * scene->tex_map.debug_spritesheet.sprite_size.y);
	image_t buffer_image = image_create(scene->tex_map.debug_spritesheet.sprite_size.x,
			scene->tex_map.debug_spritesheet.sprite_size.y, buffer);
	spritesheet_get_sprite(&scene->tex_map.debug_spritesheet, (vec2u32_t){0, 0}, &buffer_image);

	const u32 src_x = render_get_texture_x(&plane_hit, face, buffer_image.width);
	const render_texture_t tex_data = {
		// .pixels = scene->tex_map.debug.pixel_buffer,
		.pixels = scene->tex_map.debug.pixel_buffer,
		.coords = { src_x, y },
		.strip = { strip_width, strip_height },
		.size = { buffer_image.width, buffer_image.height, },
		// .size = { scene->tex_map.debug.width, scene->tex_map.debug.height, },
	};
	render_block_texture_on_image(
		&tex_data,
		x*strip_width, screen_height,
		shadow, image);
	free(buffer);
}

void render_on_image(
	const u32 x, const u32 strip_width, const u32 screen_height,
	const scene_t *scene,
	const vec2f32_t player_ray, const vec2f32_t ray,
	image_t* image
)
{
	collision_block_t cb = collision_block_empty();
	const bool hit_block = collision_hit_a_block(scene, scene->player.position, ray, &cb);

	collision_entity_t ce = collision_entity_empty();
	const bool hit_entity = collision_hit_an_entity(scene, scene->player.position, ray, &ce);

	const bool should_render_cb = hit_block && (!hit_entity || ce.dist > cb.dist);
	const bool should_render_ce = hit_entity && (!hit_block || cb.dist > ce.dist);

	if (should_render_cb) {
		render_blocks(x, strip_width, screen_height, scene, player_ray, &cb, image);
	} else if (should_render_ce) {
		// BUG: Some weird bug with entity texture (128x128)
		// Map every face?
		// Weird behavior between the entity angle and player angle
		render_entity(x, strip_width, screen_height, scene, player_ray, &ce, image);
	}
}

void* _render_slice(void* data)
{
	render_data_t *slice_data = (render_data_t*)data;

	const scene_t *scene = slice_data->base_data->scene;
	const vec2f32_t *fov_plane = slice_data->base_data->fov_plane;
	const vec2f32_t player_ray = slice_data->base_data->player_ray;

	const u32 strip_width = slice_data->base_data->strip_width;
	const u32 screen_height = slice_data->base_data->screen_height;
	image_t* image = slice_data->base_data->image;

	for (u32 x = slice_data->start; x < slice_data->end; x++) {
		vec2f32_t ray = {0};
		const f32 amount = (f32)x / (float)RENDER_WIDTH;
		vec2f32_lerp(&fov_plane[0], &fov_plane[1], amount, &ray);
		render_on_image(x, strip_width, screen_height, scene, player_ray, ray, image);
	}

	return 0;
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

	pthread_t threads[MAX_THREADS];
	render_data_t slice_data[MAX_THREADS] = {0};
	render_base_data_t base_data = {
		.scene = scene,
		.strip_width = strip_width,
		.screen_height = screen_height,
		.fov_plane = fov_plane,
		.player_ray = player_ray,
		.image = image,
	};

	const u32 step = RENDER_WIDTH / MAX_THREADS;
	for (u32 i = 0; i < MAX_THREADS; i++) {
		slice_data[i].base_data = &base_data;
		slice_data[i].start = step * i;
		slice_data[i].end = step * (i+1);
		pthread_create(&threads[i], 0, _render_slice, (void*)&slice_data[i]);
	}

	for (u32 i = 0; i < MAX_THREADS; i++) {
		pthread_join(threads[i], 0);
	}
}
