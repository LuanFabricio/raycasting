#include "image.h"
#include "types.h"

spritesheet_t spritesheet_create(const image_t *img, const vec2u32_t sprite_size)
{
	spritesheet_t sh = {
		.img_ptr = img,
		.sprite_size = sprite_size,
	};

	return sh;
}

void spritesheet_get_sprite(const spritesheet_t *spritesheet, vec2u32_t pos, image_t *dest)
{
	const bool valid_x = pos.x < spritesheet->sprite_size.x ;
	const bool valid_y = pos.y < spritesheet->sprite_size.y ;
	if (!valid_x || !valid_y) return;

	pos.x *= spritesheet->sprite_size.x;
	pos.y *= spritesheet->sprite_size.y;
	image_crop_to_buffer(spritesheet->img_ptr, pos, spritesheet->sprite_size, dest->pixel_buffer);
}
