#include <stdio.h>
#include "raylib.h"

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

image_t image_create(u32 width, u32 height, u32* pixel_buffer)
{
	return (image_t) {
		.width = width,
		.height = height,
		.pixel_buffer = pixel_buffer,
	};
}

void image_set_pixel(image_t *img, u32 x, u32 y, u32 color)
{
	const u32 index = img->width * y + x;
	img->pixel_buffer[index] = color;
}

#define SCREEN_WITDH 640
#define SCREEN_HEIGHT 640

int main(void)
{
	const box_t box1 = {
		.x = 42, .y = 42,
		.w = 32, .h = 32,
	};

	InitWindow(SCREEN_WITDH, SCREEN_HEIGHT, "RayCast");
	const i32 color = 0xff00ffff;

	u32 pixel_buffer[64 * 64] = {0};
	Texture2D tex = LoadTextureFromImage((Image){
		.mipmaps = 1,
		.width = 64,
		.height = 64,
		.data = pixel_buffer,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
	});

	image_t img = image_create(64, 64, pixel_buffer);

	for (u32 xx = 0; xx < 16; xx++)
		for (u32 yy = 0; yy < 64; yy++) image_set_pixel(&img, xx, yy, 0xffff0000);

	while(!WindowShouldClose()) {
		UpdateTexture(tex, img.pixel_buffer);

		BeginDrawing();
			DrawRectangle(box1.x, box1.y, box1.w, box1.h, *(Color*)(&color));
			DrawTexture(tex, box1.x, box1.y, WHITE);
		EndDrawing();
	}

	printf("Hello, world!\n");
	return 0;
}
