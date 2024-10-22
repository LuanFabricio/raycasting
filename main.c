#include <stdio.h>
#include <math.h>
#include "raylib.h"

#include "src/types.h"
#include "src/image.h"
#include "src/utils.h"

#define BLOCK_SIZE 64

#define SCREEN_WITDH 16*BLOCK_SIZE
#define SCREEN_HEIGHT 9*BLOCK_SIZE

int main(void)
{
	box_t box1 = {
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

	u32 pixel_buffer_test[32 * 32] = {0};
	image_t img_test = image_create(32, 32, pixel_buffer_test);
	image_draw_rectangle_color(&img_test, 0, 0, 32, 32, (0xff0000 << 8 * 1) | (0xff / 2));

	// for (u32 xx = 0; xx < 32 * 32; xx++) img_test.pixel_buffer[xx] = 0xff0000f0;

	image_t img = image_create(64, 64, pixel_buffer);

	for (u32 xx = 0; xx < 16; xx++)
		for (u32 yy = 0; yy < 64; yy++) image_set_pixel(&img, xx, yy, 0xffff0000);

	image_draw_rectangle_color(&img, 0, 0, 16, 16, 0xff0000ff);
	image_draw_rectangle_color(&img, 32, 32, 64, 64, 0xffffffff);

	image_append_image(&img, &img_test, (vec2u32_t){ .x = 4, .y = 4});

	block_e blocks[SCREEN_WITDH*SCREEN_HEIGHT / BLOCK_SIZE] = {BLOCK_EMPTY};
	scene_t scene = {
		.width = SCREEN_WITDH / BLOCK_SIZE,
		.height = SCREEN_HEIGHT / BLOCK_SIZE,
		.blocks = blocks,
	};
	scene.blocks[0] = BLOCK_BRICKS;
	scene.blocks[2] = BLOCK_BRICKS;

	u32 scene_tex_buffer[SCREEN_WITDH*SCREEN_HEIGHT] = {0xff000000};
	image_t scene_img = {
		.width = SCREEN_WITDH,
		.height = SCREEN_HEIGHT,
		.pixel_buffer = scene_tex_buffer,
	};
	Texture2D scene_tex = LoadTextureFromImage((Image){
		.mipmaps = 1,
		.width = scene_img.width,
		.height = scene_img.height,
		.data = scene_img.pixel_buffer,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
	});

	vec2f32_t speed = { 1.0f, 1.0f };
	SetTargetFPS(60);
	while(!WindowShouldClose()) {
		image_clear(&scene_img, 0xff000000);
		for (u32 yy = 0; yy < scene.height; yy++) {
			for (u32 xx = 0; xx < scene.width; xx++) {
				const u32 index = xy_to_index(xx, yy, scene.width);

				switch (scene.blocks[index]) {
					case BLOCK_BRICKS: {
						image_draw_rectangle_color(&scene_img, xx*BLOCK_SIZE, yy*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, 0xff0000ff);
					} break;
					default:
						break;
				}
			}
		}
		UpdateTexture(scene_tex, scene_img.pixel_buffer);

		image_clear(&img, 0);
		double time_sin, time_cos;
		time_sin = sin(GetTime());
		time_cos = cos(GetTime());

		const u32 img_color = (0xff << 8 * 3)
			| (u32)(0xff * time_sin) << 8 * 2
			| (u32)(0xff * time_cos) << 8 * 1
			| (u32)(0xff * GetFrameTime()) << 8 * 0;
		image_draw_rectangle_color(&img, 0, 0, box1.w, box1.h, img_color);

		UpdateTexture(tex, img.pixel_buffer);

		BeginDrawing();
			ClearBackground(BLACK);
			// DrawRectangle(box1.x, box1.y, box1.w, box1.h, *(Color*)(&color));
			DrawTexture(tex, box1.x, box1.y, WHITE);
			DrawTexture(scene_tex, 0, 0, WHITE);
		EndDrawing();

		f32 delta_time = GetFrameTime();
		box1.x += speed.x * delta_time * 100;
		if (box1.x <= 0 || box1.x + box1.w >= SCREEN_WITDH) speed.x = -speed.x;
		box1.y += speed.y * delta_time * 100;
		if (box1.y <= 0 || box1.y + box1.h >= SCREEN_HEIGHT) speed.y = -speed.y;
	}

	printf("Hello, world!\n");
	return 0;
}
