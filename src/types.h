#ifndef __SRC_TYPES_H__
#define __SRC_TYPES_H__

#include <stdbool.h>

#include "defines.h"

typedef float f32;
typedef double f64;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long int u64;
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
	image_t *img_ptr;
	vec2u32_t sprite_size;
	vec2u32_t sprite_amount;
} spritesheet_t;

typedef struct {
	spritesheet_t spritesheet;
	vec2u32_t sprites_len;
	vec2u32_t current_sprite;
	u32 timer;
	u32 delay;
} sprite_animation_t;

typedef struct {
	f32 x, y;
	u32 w, h;
} box_t;

typedef struct {
	const u32 *pixels;
	vec2i32_t coords;
	vec2f32_t strip;
	vec2u32_t size;
} render_texture_t;

typedef enum { BLOCK_EMPTY, BLOCK_COLOR, BLOCK_BRICKS } block_type_e;

typedef enum {
	BLOCK_FACE_UP	 = 0x00,
	BLOCK_FACE_RIGHT = 0x01,
	BLOCK_FACE_DOWN	 = 0x02,
	BLOCK_FACE_LEFT	 = 0x03,
	BLOCK_FACE_NONE	 = 0xff,
} block_face_e;


typedef enum {
	PORTAL_NONE	= 0b0,
	PORTAL_1 	= 0b1,
	PORTAL_2 	= 0b10,
} portal_e;

typedef enum {
	COLOR_CHANNEL_ALPHA = 3,
	COLOR_CHANNEL_BLUE = 2,
	COLOR_CHANNEL_GREEN = 1,
	COLOR_CHANNEL_RED = 0,
} color_channel_e;

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

typedef struct {
	const block_t* block_src;
	const block_t* block_dest;
	vec2u32_t position;
	block_face_e face;
	u32 *pixels;
} portal_t;

typedef enum {
	ENTITY_PLAYER	= 0x00,
	ENTITY_ENEMY	= 0x01,
} entity_e;

typedef struct {
	vec2f32_t position;
	f32 angle;
	entity_e type;
	i32 hp;
} entity_t;

typedef enum {
	ENTITY_SPRITE_ANIMATION	  = 0x00,
	ENTITY_SPRITE_SPRITESHEET = 0x01,
} entity_sprite_e;

typedef struct {
	entity_sprite_e type;
	void* sprite;
} entity_sprite_t;

typedef struct {
	entity_t data[MAX_ENTITIES];
	entity_sprite_t sprites[MAX_ENTITIES];
	u32 lenght;
} entity_array_t;

typedef struct {
	image_t bricks_green;
	image_t cross_blue;
	image_t brick_img;
	image_t portal1;
	image_t portal2;
	image_t debug;
	image_t debug_spritesheet_img;
	spritesheet_t debug_spritesheet;
	sprite_animation_t animation;
} texture_map_t;

typedef struct {
	u32 blue;
	u32 red;
	u32 green;
	u32 white;
	u32 grey;
} color_map_t;

typedef struct {
	u32 width, height;
	block_t blocks[SCENE_WIDTH*SCENE_HEIGHT];
	entity_t player;
	portal_t portal1;
	portal_t portal2;
	// TODO: Add entites (NPCs or enemies)
	entity_array_t entities;
	texture_map_t tex_map;
	color_map_t color_map;
} scene_t;

typedef struct {
	const block_t *block_ptr;
	block_face_e face;
	vec2f32_t hit;
	vec2u32_t position;
	f32 dist;
} collision_block_t;

typedef struct {
	entity_t *entity_ptr;
	vec2f32_t hit;
	f32 dist;
	u32 index;
} collision_entity_t;

typedef struct {
	const u32 strip_width;
	const u32 screen_height;
	scene_t* scene;

	const vec2f32_t *fov_plane;
	const vec2f32_t player_ray;

	image_t* image;
} render_base_data_t;

typedef struct {
	u32 start;
	u32 end;
	render_base_data_t *base_data;
} render_data_t;

#endif // __SRC_TYPES_H__
