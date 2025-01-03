#ifndef __SRC_SCENE_H__
#define __SRC_SCENE_H__

#include "types.h"

void scene_get_block_points(u32 x, u32 y, float scale, vec2f32_t *out);
void scene_teleport_player(scene_t *scene, const collision_block_t *collision_block);
void scene_place_teleport(scene_t *scene, portal_e portal_type);
void scene_move_player(scene_t *scene, vec2f32_t speed);
void scene_rotate_player_camera(scene_t *scene, const f32 angle);

#endif // __SRC_SCENE_H__
