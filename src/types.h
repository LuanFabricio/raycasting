#ifndef __SRC_TYPES_H__
#define __SRC_TYPES_H__

typedef float f32;
typedef double f64;
typedef unsigned int u32;
typedef int i32;

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

typedef struct {
	image_t image_buffer;
} scene_t;

#endif // __SRC_TYPES_H__
