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

void scene_teleport_player(scene_t *scene, const block_t *block)
{
		portal_t *portal = (block->portal == PORTAL_1) ? &scene->portal1 : &scene->portal2;
		const f32 angle = update_speed_direction(portal);

		switch (block->portal) {
			case PORTAL_1: {
				scene->player_position.x = scene->portal2.position.x;
				scene->player_position.y = scene->portal2.position.y;
			} break;
			case PORTAL_2: {
				scene->player_position.x = scene->portal1.position.x;
				scene->player_position.y = scene->portal1.position.y;
			} break;

			default:
				break;

		}

		scene->player_angle += 2*angle;
		vec2f32_t speed_mul = vec2f32_from_angle(angle);
		// BUG: The player position after left the portal isn't right
		scene->player_position.x += 1.2f * speed_mul.x + (1.0f - speed_mul.x) * 0.5;
		scene->player_position.y += 1.2f * speed_mul.y + (1.0f - speed_mul.y) * 0.5;
}

void scene_place_teleport(scene_t *scene)
{
	vec2f32_t fov[2] = {0};
	get_fov_plane(scene->player_position, scene->player_angle, FAR_DISTANCE, fov);

	vec2f32_t ray = vec2f32_from_angle(scene->player_angle);
	vec2f32_lerp(&fov[0], &fov[1], 0.5f, &ray);

	collision_block_t collision_block = collision_block_empty();
	bool have_collision = collision_hit_a_block(scene, scene->player_position, ray, &collision_block);

	if (!have_collision) return;

	collision_block.block_ptr->portal = PORTAL_1;
	collision_block.block_ptr->portal_face = collision_block.face;

	scene->portal2.block_dest = collision_block.block_ptr;
}
