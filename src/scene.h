#ifndef __SRC_SCENE_H__
#define __SRC_SCENE_H__

#include "types.h"

void scene_get_block_points(u32 x, u32 y, float scale, vec2f32_t *out);
vec2f32_t scene_offset_to_center_block_face(const vec2f32_t *position, const block_face_e face, f32 offset_scale);
void scene_teleport_player(scene_t *scene, const collision_block_t *collision_block);
void scene_place_teleport(scene_t *scene, portal_e portal_type);
void scene_move_player(scene_t *scene, vec2f32_t speed);
void scene_rotate_player_camera(scene_t *scene, const f32 angle);
void scene_player_shoot(scene_t *scene);

#endif // __SRC_SCENE_H__
