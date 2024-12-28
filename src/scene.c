#include <assert.h>

#include "defines.h"
#include "types.h"
#include "vec2f32.h"
#include "utils.h"
#include "render.h"

#include "scene.h"
#include "collision.h"

void scene_get_block_points(u32 x, u32 y, float scale, vec2f32_t *out)
{
	// (x, y)
	out[0].x = x 	 * scale;
	out[0].y = y 	 * scale;
	// (x+1, y)
	out[1].x = (x+1) * scale;
	out[1].y = y 	 * scale;
	// (x+1, y+1)
	out[2].x = (x+1) * scale;
	out[2].y = (y+1) * scale;
	// (x, y+1)
	out[3].x = x	 * scale;
	out[3].y = (y+1) * scale;
}

void scene_teleport_player(scene_t *scene, const collision_block_t *collision_block)
{
	const bool hit_portal1_block = collision_block->block_ptr == scene->portal1.block_src;
	const bool hit_portal1_face = collision_block->face == scene->portal1.face;

	const bool hit_portal2_block = collision_block->block_ptr == scene->portal2.block_src;
	const bool hit_portal2_face = collision_block->face == scene->portal2.face;

	portal_t *portal_src = 0;
	portal_t *portal_dest = 0;

	if (hit_portal1_block && hit_portal1_face) {
		portal_src = &scene->portal1;
		portal_dest = &scene->portal2;
	} else if (hit_portal2_block && hit_portal2_face) {
		portal_src = &scene->portal2;
		portal_dest = &scene->portal1;
	} else {
		return;
	}

	if (portal_dest->block_src == 0 || portal_src->block_dest == 0) return;

	assert(portal_src != 0 && "It should be a valid pointer to a portal.");
	assert(portal_dest != 0 && "It should be a valid pointer to a portal.");

	entity_t *player_ptr = &scene->player;
	player_ptr->position.x = portal_dest->position.x + 0.5f;
	player_ptr->position.y = portal_dest->position.y + 0.5f;

	const f32 angle = update_player_angle(portal_src, portal_dest);
	player_ptr->angle += angle;

	// TODO: Use the momentum instead of this offset
	const f32 offset = 0.75f;
	block_face_e dest_face = portal_dest->face;
	switch (dest_face) {
		case BLOCK_FACE_UP: {
			player_ptr->position.y -= offset;
		} break;
		case BLOCK_FACE_RIGHT: {
			player_ptr->position.x += offset;
		} break;
		case BLOCK_FACE_DOWN: {
			player_ptr->position.y += offset;
		} break;
		case BLOCK_FACE_LEFT: {
			player_ptr->position.x -= offset;
		} break;
		default:
				      break;
	}
}

void scene_place_teleport(scene_t *scene, portal_e portal_type)
{
	entity_t *player_ptr = &scene->player;
	vec2f32_t fov[2] = {0};
	get_fov_plane(player_ptr->position, player_ptr->angle, FAR_DISTANCE, fov);

	vec2f32_t ray = vec2f32_from_angle(player_ptr->angle);
	vec2f32_lerp(&fov[0], &fov[1], 0.5f, &ray);

	collision_block_t collision_block = collision_block_empty();
	bool have_collision = collision_hit_a_block(scene, player_ptr->position, ray, &collision_block);

	if (!have_collision) return;

	portal_t *portal_src = 0;
	portal_t *portal_dest = 0;

	// portal_e portal_type = PORTAL_1;
	switch (portal_type) {
		case PORTAL_1: {
			portal_src = &scene->portal1;
			portal_dest = &scene->portal2;
		} break;

		case PORTAL_2: {
			portal_src = &scene->portal2;
			portal_dest = &scene->portal1;
		} break;

		default:
		       return;
	}

	portal_src->block_src = collision_block.block_ptr;
	portal_src->position = collision_block.position;
	portal_src->face = collision_block.face;

	const bool same_block = portal_src->block_src == portal_dest->block_src;
	const bool same_face = portal_src->face == portal_dest->face;

	if (same_block && same_face) {
		portal_dest->block_src = 0;
		portal_dest->face = BLOCK_FACE_NONE;
		portal_src->block_dest = 0;
	} else {
		portal_src->block_dest = portal_dest->block_src;
		portal_dest->block_dest = collision_block.block_ptr;
	}
}

void scene_move_player(scene_t *scene, const vec2f32_t speed)
{
	const f32 max_x = scene->width;
	const f32 new_x = scene->player.position.x + speed.x;

	const f32 max_y = scene->height;
	const f32 new_y = scene->player.position.y + speed.y;

	vec2f32_t new_position = {
		.x = MAX(MIN(new_x, max_x), 0),
		.y = scene->player.position.y,
	};
	collision_block_t collision_block = collision_block_empty();
	bool hit_a_block = collision_hit_a_block(scene, scene->player.position, new_position, &collision_block);

	bool is_a_portal_block = collision_block.block_ptr == scene->portal1.block_src
		|| collision_block.block_ptr == scene->portal2.block_src;
	if (!hit_a_block) {
		scene->player.position.x = new_position.x;
	}
	else if (is_a_portal_block) {
		scene_teleport_player(scene, &collision_block);
		return;
	} else {
		switch (collision_block.face) {
			case BLOCK_FACE_RIGHT:
				collision_block.hit.x += 0.1;
				break;
			case BLOCK_FACE_LEFT:
				collision_block.hit.x -= 0.1;
				break;
			default:
				break;
		}
		scene->player.position.x = collision_block.hit.x;
	}

	new_position.x = scene->player.position.x;
	new_position.y = MAX(MIN(new_y, max_y), 0);
	hit_a_block = collision_hit_a_block(scene, scene->player.position, new_position, &collision_block);

	is_a_portal_block = collision_block.block_ptr == scene->portal1.block_src
		|| collision_block.block_ptr == scene->portal2.block_src;
	if (!hit_a_block) {
		scene->player.position.y = new_position.y;
	}
	else if (is_a_portal_block) {
		scene_teleport_player(scene, &collision_block);
		return;
	} else {
		switch (collision_block.face) {
			case BLOCK_FACE_UP:
				collision_block.hit.y -= 0.1;
				break;
			case BLOCK_FACE_DOWN:
				collision_block.hit.y += 0.1;
				break;
			default:
				break;
		}
		scene->player.position.y = collision_block.hit.y;
	}
}

void scene_rotate_player_camera(scene_t *scene, const f32 angle)
{
	f32 new_angle = scene->player.angle + angle;

	if (new_angle < 0) {
		new_angle = 2 * PI - scene->player.angle;
	} else if (new_angle > 2 * PI) {
		new_angle = scene->player.angle - 2 * PI;
	}

	scene->player.angle = new_angle;
}
