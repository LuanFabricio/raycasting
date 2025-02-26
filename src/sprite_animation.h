#ifndef __SRC_SPRITE_ANIMATION_H__
#define __SRC_SPRITE_ANIMATION_H__
#include "types.h"

sprite_animation_t sprite_animation_create(const spritesheet_t spritesheet, const u32 delay);
void sprite_animation_next_sprite(sprite_animation_t *sprite_animation);
bool sprite_animation_should_update(sprite_animation_t* sprite_animation);
void sprite_animation_image(const sprite_animation_t* sprite_animation, image_t *buffer);

#endif // __SRC_SPRITE_ANIMATION_H__
