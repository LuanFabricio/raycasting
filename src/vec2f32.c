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
	f32 len = sqrt(v->x * v->x + v->y * v->y);
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
