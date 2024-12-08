#ifndef __SRC_COLLISION_H__
#define __SRC_COLLISION_H__

#include <stdbool.h>
#include "types.h"

bool collision_intersects(vec2f32_t s1, vec2f32_t e1, vec2f32_t s2, vec2f32_t e2, vec2f32_t *out);
bool collision_hit_a_block(const scene_t *scene, const vec2f32_t p1, const vec2f32_t p2, vec2f32_t *hit, block_t **block, block_face_e *block_face);
bool collision_hit_ceil(const scene_t *scene, const vec2f32_t p1, const vec2f32_t p2, vec2f32_t *hit, ceil_e *ceil);
i32 collision_point_in_block(const block_t* blocks, const u32 blocks_len, const u32 width, vec2f32_t point);

#endif // __SRC_COLLISION_H__
