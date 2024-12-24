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
	portal_t *portal_src = 0;
	portal_t *portal_dest = 0;
	if (collision_block->block_ptr == scene->portal1.block_src && collision_block->face == scene->portal1.face) {
		scene->player_position.x = scene->portal2.position.x;
		scene->player_position.y = scene->portal2.position.y;
		portal_src = &scene->portal1;
		portal_dest = &scene->portal2;
	} else {

		scene->player_position.x = scene->portal1.position.x;
		scene->player_position.y = scene->portal1.position.y;
		portal_src = &scene->portal2;
		portal_dest = &scene->portal1;
	}

	assert(portal_src != 0 && "It should be a pointer to a portal.");
	assert(portal_dest != 0 && "It should be a pointer to a portal.");

	scene->player_position.x += 0.5f;
	scene->player_position.y += 0.5f;

	const f32 angle = update_player_angle(portal_src, portal_dest);
	scene->player_angle += angle;

	// TODO: Use the momentum instead of this offset
	const f32 offset = 0.75f;
	block_face_e dest_face = portal_dest->face;
	switch (dest_face) {
		case BLOCK_FACE_UP: {
					    scene->player_position.y -= offset;
				    } break;
		case BLOCK_FACE_RIGHT: {
					       scene->player_position.x += offset;
				       } break;
		case BLOCK_FACE_DOWN: {
					      scene->player_position.y += offset;
				      } break;
		case BLOCK_FACE_LEFT: {
					      scene->player_position.x -= offset;
				      } break;
		default:
				      break;
	}
}

void scene_place_teleport(scene_t *scene, portal_e portal_type)
{
	vec2f32_t fov[2] = {0};
	get_fov_plane(scene->player_position, scene->player_angle, FAR_DISTANCE, fov);

	vec2f32_t ray = vec2f32_from_angle(scene->player_angle);
	vec2f32_lerp(&fov[0], &fov[1], 0.5f, &ray);

	collision_block_t collision_block = collision_block_empty();
	bool have_collision = collision_hit_a_block(scene, scene->player_position, ray, &collision_block);

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

	portal_dest->block_dest = collision_block.block_ptr;

	portal_src->block_src = collision_block.block_ptr;
	portal_src->position = collision_block.position;
	portal_src->block_dest = portal_dest->block_src;
	portal_src->face = collision_block.face;
}
