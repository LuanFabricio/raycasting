#ifndef _SRC_VEC2F32_H_
#define _SRC_VEC2F32_H_

#include "types.h"

vec2f32_t vec2f32_from_angle(f32 angle);
void vec2f32_norm(const vec2f32_t *v, vec2f32_t *dest);
void vec2f32_rot90(const vec2f32_t *v, vec2f32_t *dest);
void vec2f32_add(const vec2f32_t *v1, const vec2f32_t *v2, vec2f32_t *dest);
void vec2f32_sub(const vec2f32_t *v1, const vec2f32_t *v2, vec2f32_t *dest);
void vec2f32_scale(const vec2f32_t *v, f32 scale, vec2f32_t *dest);
void vec2f32_lerp(const vec2f32_t *v1, const vec2f32_t *v2, f32 amount, vec2f32_t *dest);
void vec2f32_ceil(const vec2f32_t *v, vec2f32_t *dest);
void vec2f32_floor(const vec2f32_t *v, vec2f32_t *dest);
f32 vec2f32_dot(const vec2f32_t *v1, const vec2f32_t *v2);
f32 vec2f32_length(const vec2f32_t* v);
f32 vec2f32_distance(const vec2f32_t *v1, const vec2f32_t *v2);
void vec2f32_copy_signal(const vec2f32_t *v1, const vec2f32_t *v2, vec2f32_t* dest);

#endif // _SRC_VEC2F32_H_
