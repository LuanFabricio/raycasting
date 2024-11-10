#ifndef __SRC_COLLISION_H__
#define __SRC_COLLISION_H__

#include <stdbool.h>
#include "types.h"

bool collision_intersects(vec2f32_t s1, vec2f32_t e1, vec2f32_t s2, vec2f32_t e2, vec2f32_t *out);
bool collision_hit_a_block(const scene_t *scene, const vec2f32_t p1, const vec2f32_t p2, vec2f32_t *hit);
i32 collision_point_in_block(const block_e* blocks, const u32 blocks_len, const u32 width, vec2f32_t point);
void get_block_points(u32 x, u32 y, float scale, vec2f32_t *out);

#endif // __SRC_COLLISION_H__
