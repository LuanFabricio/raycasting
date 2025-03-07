#include <math.h>

#include "types.h"
#include "utils.h"
#include "vec2f32.h"

vec2f32_t vec2f32_from_angle(f32 angle)
{
	return (vec2f32_t) {
		.x = cos(angle),
		.y = sin(angle),
	};
}

void vec2f32_norm(const vec2f32_t *v, vec2f32_t *dest)
{
	f32 len = vec2f32_length(v);
	dest->x /= len;
	dest->y /= len;
}

void vec2f32_rot90(const vec2f32_t *v, vec2f32_t *dest)
{
	f32 x = -v->y;
	f32 y = v->x;

	dest->x = x;
	dest->y = y;
}

void vec2f32_rot(const vec2f32_t *src, const f32 angle, vec2f32_t *dest)
{
	const f32 x = src->x * cosf(angle) - src->y * sinf(angle);
	const f32 y = src->x * sinf(angle) + src->y * cosf(angle);

	dest->x = x;
	dest->y = y;
}

void vec2f32_add(const vec2f32_t *v1, const vec2f32_t *v2, vec2f32_t *dest)
{
	dest->x = v1->x + v2->x;
	dest->y = v1->y + v2->y;
}

void vec2f32_sub(const vec2f32_t *v1, const vec2f32_t *v2, vec2f32_t *dest)
{
	dest->x = v1->x - v2->x;
	dest->y = v1->y - v2->y;
}

void vec2f32_scale(const vec2f32_t *v, f32 scale, vec2f32_t *dest)
{
	dest->x = v->x * scale;
	dest->y = v->y * scale;
}

void vec2f32_lerp(const vec2f32_t *v1, const vec2f32_t *v2, f32 amount, vec2f32_t *dest)
{
	dest->x = lerp(v1->x, v2->x, amount);
	dest->y = lerp(v1->y, v2->y, amount);
}

void vec2f32_ceil(const vec2f32_t *v, vec2f32_t *dest)
{
	dest->x = ceilf(v->x);
	dest->y = ceilf(v->y);
}

void vec2f32_floor(const vec2f32_t *v, vec2f32_t *dest)
{
	dest->x = floorf(v->x);
	dest->y = floorf(v->y);
}

f32 vec2f32_dot(const vec2f32_t *v1, const vec2f32_t *v2)
{
	return v1->x * v2->x + v1->y * v2->y;
}

f32 vec2f32_length(const vec2f32_t* v)
{
	f32 x2 = v->x * v->x;
	f32 y2 = v->y * v->y;
	return sqrtf(x2 + y2);
}

f32 vec2f32_distance(const vec2f32_t *v1, const vec2f32_t *v2)
{
	f32 dx = v1->x - v2->x;
	f32 dy = v1->y - v2->y;

	return sqrt(dx*dx + dy*dy);
}

void vec2f32_copy_signal(const vec2f32_t *v1, const vec2f32_t *v2, vec2f32_t* dest)
{
	dest->x = copysignf(v2->x, v1->x);
	dest->y = copysignf(v2->y, v1->y);
}
