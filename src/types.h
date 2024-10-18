#ifndef __SRC_TYPES_H__
#define __SRC_TYPES_H__

typedef unsigned int u32;
typedef int i32;

typedef struct {
	u32 width, height;
	u32 *pixel_buffer;
} image_t;

typedef struct {
	i32 x, y;
	u32 w, h;
} box_t;

typedef struct {
	image_t image_buffer;
} scene_t;


#endif // __SRC_TYPES_H__
