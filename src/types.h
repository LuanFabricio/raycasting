#ifndef __SRC_TYPES_H__
#define __SRC_TYPES_H__

typedef float f32;
typedef double f64;
typedef unsigned char u8;
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
	i32 x, y;
} vec2i32_t;

typedef struct {
	u32 width, height;
	u32 *pixel_buffer;
} image_t;

typedef struct {
	f32 x, y;
	u32 w, h;
} box_t;

typedef struct {
	u32 *pixels;
	vec2i32_t coords;
	vec2f32_t strip;
	vec2u32_t size;
} render_texture_t;

typedef enum { BLOCK_EMPTY, BLOCK_COLOR, BLOCK_BRICKS } block_type_e;

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
	// NOTE: Either a ptr to a color or a ptr to a texture.
	// The default texture size is 64 x 64
	void* data;
} block_t;

typedef enum { CEIL_NONE, CEIL_BLUE, CEIL_RED } ceil_e;

typedef struct {
	u32 width, height;
	block_t *blocks;
	ceil_e *ceil_grid;
	vec2f32_t player_position;
	f32 player_angle;
} scene_t;

#endif // __SRC_TYPES_H__
