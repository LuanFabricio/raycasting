#include "types.h"
#include "vec2f32.h"
#include "utils.h"

#include "scene.h"

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
		vec2f32_t speed_mul = vec2f32_from_angle(angle);
		vec2f32_norm(&speed_mul, &speed_mul);

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

		// TODO: Check if its the best way
		scene->player_angle += 2*angle;
		scene->player_position.x += 1.2f * speed_mul.x + (1.0f - speed_mul.x) * 0.5;
		scene->player_position.y += 1.2f * speed_mul.y + (1.0f - speed_mul.y) * 0.5;
}
