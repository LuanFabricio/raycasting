#include <float.h>

#include "types.h"
#include "defines.h"

#include "collision.h"
#include "scene.h"
#include "utils.h"
#include "vec2f32.h"

collision_block_t collision_block_empty()
{
	return (collision_block_t) {
		.block_ptr = 0,
		.face = BLOCK_FACE_NONE,
		.hit = {0},
		.position = {0},
	};
}

bool collision_block_match_portal_face(const collision_block_t *collision_block)
{
	return collision_block->block_ptr && collision_block->face == collision_block->block_ptr->portal_face;
}

bool collision_block_is_portal_face_none(const collision_block_t *collision_block)
{
	return collision_block->block_ptr && collision_block->block_ptr->portal_face == BLOCK_FACE_NONE;
}

bool collision_block_is_portal_none(const collision_block_t *collision_block)
{
	return collision_block->block_ptr && collision_block->block_ptr->portal == PORTAL_NONE;
}

bool collision_intersects(vec2f32_t s1, vec2f32_t e1, vec2f32_t s2, vec2f32_t e2, vec2f32_t *out)
{
	f32 dx = e1.x - s1.x;
	f32 dy = e1.y - s1.y;

	bool vertical1 = dx == 0.0;
	f32 a1 = vertical1 ? 0.0 : dy / dx;
	f32 b1 = e1.y - a1 * e1.x;

#ifdef LOG
	printf("P1:\n");
	printf("\tdx: %.02f\n", dx);
	printf("\tdy: %.02f\n", dy);
	printf("\talpha: %.02f\n", a1);
	printf("\tlin: %.02f\n", b1);
	printf("\tvertical: %b\n", vertical1);
#endif // LOG

	dx = e2.x - s2.x;
	dy = e2.y - s2.y;

	bool vertical2 = dx == 0.0;
	f32 a2 = vertical2 ? 0.0 : dy / dx;
	f32 b2 = e2.y - a2 * e2.x;

#ifdef LOG
	printf("P2:\n");
	printf("\tdx: %.02f\n", dx);
	printf("\tdy: %.02f\n", dy);
	printf("\talpha: %.02f\n", a2);
	printf("\tlin: %.02f\n", b2);
	printf("\tvertical: %b\n", vertical2);
#endif // LOG

	if (a1 == a2 && vertical1 == vertical2) return false;
	if (vertical1 && vertical2) return false;

	if (vertical1) {
		out->x = s1.x;
		out->y = a2*out->x + b2;
	} else if (vertical2) {
		out->x = s2.x;
		out->y = a1*out->x + b1;
	} else {
		out->x = (b2 - b1) / (a1 - a2);
		out->y = a1*out->x + b1;
	}

#ifdef LOG
	printf("Out: %.05f, %.05f\n", out->x, out->y);
#endif // LOG

	// Line 1
	f32 min_x = s1.x < e1.x ? s1.x : e1.x;
	f32 max_x = s1.x > e1.x ? s1.x : e1.x;

	f32 min_y = s1.y < e1.y ? s1.y : e1.y;
	f32 max_y = s1.y > e1.y ? s1.y : e1.y;

#ifdef LOG
	printf("Out: %.05f, %.05f\n", out->x, out->y);
	printf("P1 min/max x: %.05f, %.05f\n", min_x, max_x);
	printf("P1 min/max y: %.05f, %.05f\n", min_y, max_y);
#endif // LOG

	if ((min_x - out->x) > EPSILON || (out->x - max_x) > EPSILON) return false;
	if ((min_y - out->y) > EPSILON || (out->y - max_y) > EPSILON) return false;
	// Line 2
	min_x = s2.x < e2.x ? s2.x : e2.x;
	max_x = s2.x > e2.x ? s2.x : e2.x;

	min_y = s2.y < e2.y ? s2.y : e2.y;
	max_y = s2.y > e2.y ? s2.y : e2.y;

#ifdef LOG
	printf("P2 min/max x: %.05f, %.05f\n", min_x, max_x);
	printf("P2 min/max y: %.05f, %.05f\n", min_y, max_y);
#endif // LOG

	if ((min_x - out->x) > EPSILON || (out->x - max_x) > EPSILON) return false;
	if ((min_y - out->y) > EPSILON || (out->y - max_y) > EPSILON) return false;

	return true;
}

i32 collision_point_in_block(const block_t* blocks, const u32 blocks_len, const u32 width, vec2f32_t point)
{
	for (u32 i = 0; i < blocks_len; i++) {
		block_type_e block = blocks[i].block_type;

		if (block != BLOCK_EMPTY) {
			vec2u32_t min_point = index_to_xy(i, width);
			min_point.x *= BLOCK_SIZE;
			min_point.y *= BLOCK_SIZE;

			vec2u32_t max_point = {
				.x = min_point.x + BLOCK_SIZE,
				.y = min_point.y + BLOCK_SIZE,
			};

			const bool x_match = min_point.x <= point.x && point.x <= max_point.x;
			const bool y_match = min_point.y <= point.y && point.y <= max_point.y;

			if (x_match && y_match) {
				return i;
			}
		}
	}

	return -1;
}

bool collision_hit_a_block(const scene_t *scene, const vec2f32_t p1, const vec2f32_t p2, collision_block_t* collision_block)
{

	vec2f32_t points[4] = {0};
	const u32 blocks_len = scene->width * scene->height;

	const u32 lines[] = { 0, 1, 1, 2, 2, 3, 3, 0};
	bool have_hit = false;

	f32 dist = FLT_MAX;
	vec2f32_t current_hit = {};

	for (u32 i = 0; i < blocks_len; i++) {
		if (scene->blocks[i].block_type == BLOCK_EMPTY) continue;
		vec2u32_t block_pos = index_to_xy(i, scene->width);
		scene_get_block_points(block_pos.x, block_pos.y, 1.0f, points);
		for (u32 j = 0; j < sizeof(lines)/sizeof(u32); j+=2) {
#ifdef LOG
			printf("[%u, %u->%u]:\n", i, j, j+1);
			printf("\t(%.02f, %.02f) -> (%.02f, %.02f)\n", p1.x, p1.y, p2.x, p2.y);
			printf("\t(%.02f, %.02f) -> (%.02f, %.02f)\n", points[lines[j]].x, points[lines[j]].y, points[lines[j+1]].x, points[lines[j+1]].y);
#endif // LOG
			if (collision_intersects(p1, p2, points[lines[j]], points[lines[j+1]], &current_hit)) {
				f32 current_dist = vec2f32_distance(&p1, &current_hit);
				if (current_dist < dist) {
					dist = current_dist;
					if (collision_block != 0) {
						collision_block->hit = current_hit;
						collision_block->position = block_pos;
						collision_block->block_ptr = &scene->blocks[i];
						collision_block->face = j >> 1;
					}
				}

				have_hit = true;
			}
		}
	}

	return have_hit;
}

bool collision_hit_ceil(const scene_t *scene, const vec2f32_t p1, const vec2f32_t p2, vec2f32_t *hit, ceil_e *ceil)
{
	return false;
}
