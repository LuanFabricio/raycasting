#include <time.h>

#include "types.h"
#include "spritesheet.h"
#include "sprite_animation.h"

sprite_animation_t sprite_animation_create(
	spritesheet_t spritesheet,
	const u32 delay)
{
	sprite_animation_t sprite_animation = {
		.spritesheet = spritesheet,
		.delay = delay,
		.timer = time(0) + delay,
		.sprites_len = spritesheet.sprite_amount,
		.current_sprite = {0, 0},
	};

	return sprite_animation;
}

void sprite_animation_next_sprite(sprite_animation_t *sprite_animation)
{
	if (!sprite_animation_should_update(sprite_animation)) {
		return;
	}

	u32 x = sprite_animation->current_sprite.x + 1;
	u32 y = sprite_animation->current_sprite.y;

	if (x >= sprite_animation->sprites_len.x) {
		x = 0;
		y += 1;
	}

	if (y >= sprite_animation->sprites_len.y) {
		y = 0;
	}

	sprite_animation->current_sprite.x = x;
	sprite_animation->current_sprite.y = y;

	sprite_animation->timer = time(0) + sprite_animation->delay;
}

bool sprite_animation_should_update(
	sprite_animation_t* sprite_animation)
{
	return sprite_animation->timer < time(0);
}

void sprite_animation_image(
		const sprite_animation_t* sprite_animation,
		image_t *buffer)
{
	spritesheet_get_sprite(
			&sprite_animation->spritesheet, sprite_animation->current_sprite, buffer);
}
