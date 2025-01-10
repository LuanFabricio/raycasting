#ifndef __SRC_SPRITESHEET_H__
#define __SRC_SPRITESHEET_H__

#include "types.h"

spritesheet_t spritesheet_create(const image_t *img, const vec2u32_t sprite_size);
void spritesheet_get_sprite(const spritesheet_t *spritesheet, const vec2u32_t pos, image_t *dest);

#endif // __SRC_SPRITESHEET_H__
