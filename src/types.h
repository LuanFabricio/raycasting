#ifndef __SRC_TYPES_H__
#define __SRC_TYPES_H__

typedef float f32;
typedef double f64;
typedef unsigned int u32;
typedef int i32;

#ifndef __bool_true_false_are_defined
#define bool bool
#define false 0
#define true 1
#endif // bool

typedef struct {
	f32 x, y;
} vec2f32_t;

typedef struct {
	u32 x, y;
} vec2u32_t;

typedef struct {
	u32 width, height;
	u32 *pixel_buffer;
} image_t;

typedef struct {
	f32 x, y;
	u32 w, h;
} box_t;

typedef enum { BLOCK_EMPTY, BLOCK_BRICKS } block_type_e;

// TODO: Define how to handle multiple render types
// Example:
// 	If a block have a sprite, use a ptr to that sprite.
// 	If a block have a color, render the color.
// 	Maybe use a a ptr approach
// 	...
// 	block_type_e block_type;
// 	void *render_ptr;
// 	...
//	switch (block_type) {
//		case BLOCK_BRICKS:
//			render_color(render_ptr);
//			break;
//		case BLOCK_SPRITE:
//			render_sprite(render_ptr);
//			break;
//		default:
//			break;
//	}
typedef struct {
	block_type_e block_type;
	u32 color;
} block_t;

typedef struct {
	u32 width, height;
	block_t *blocks;
	vec2f32_t player_position;
	f32 player_angle;
} scene_t;

#endif // __SRC_TYPES_H__
