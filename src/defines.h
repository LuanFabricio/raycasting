#ifndef __SRC_DEFINES_H__
#define __SRC_DEFINES_H__

#define BLOCK_SIZE 8

#define SCALE 80

#define SCREEN_WITDH 16*SCALE
#define SCREEN_HEIGHT 9*SCALE

#define PLAYER_SPEED 10

#define MIN(x, y) (x) > (y) ? (y) : (x)
#define MAX(x, y) (x) < (y) ? (y) : (x)

#define BASE_ROTATION (-PI / 2)

#define FAR_DISTANCE  16.0
#define NEAR_DISTANCE 1.0
#define FOV (-PI / 2) // -90°

#define SCENE_WIDTH 16
#define SCENE_HEIGHT 16

#define EPSILON 1e-3

#endif // __SRC_DEFINES_H__
