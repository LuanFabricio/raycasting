#ifndef __SRC_COLLISION_H__
#define __SRC_COLLISION_H__

#include <stdbool.h>
#include "types.h"

collision_block_t collision_block_empty();
collision_entity_t collision_entity_empty();
bool collision_intersects(vec2f32_t s1, vec2f32_t e1, vec2f32_t s2, vec2f32_t e2, vec2f32_t *out);
bool collision_hit_a_block(const scene_t *scene, const vec2f32_t p1, const vec2f32_t p2, collision_block_t* collision_block);
bool collision_hit_an_entity(const scene_t *scene, const vec2f32_t p1, const vec2f32_t p2, collision_entity_t *collision_entity);
i32 collision_point_in_block(const block_t* blocks, const u32 blocks_len, const u32 width, vec2f32_t point);

#endif // __SRC_COLLISION_H__
