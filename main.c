#include <stdio.h>
#include <math.h>
#include "raylib.h"

#include "src/types.h"
#include "src/utils.h"
// #include "src/image.h"
// #include "src/utils.h"

#define BLOCK_SIZE 16

#define SCALE 80

#define SCREEN_WITDH 16*SCALE
#define SCREEN_HEIGHT 9*SCALE

#define PLAYER_SPEED 100

#define MIN(x, y) (x) > (y) ? (y) : (x)
#define MAX(x, y) (x) < (y) ? (y) : (x)


void draw_grid(const block_e *blocks, u32 width, u32 height)
{
	for (u32 yy = 0; yy < height; yy++) {
		for (u32 xx = 0; xx < width; xx++) {
			const u32 index = xy_to_index(xx, yy, width);

			if (blocks[index] == BLOCK_BRICKS) {
				DrawRectangle(xx*BLOCK_SIZE, yy*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, RED);
			}
		}
	}
}

void draw_player_view(const Vector2 pos, f32 angle)
{
	const f32 cone_angle_padding =  -PI / 4; // -45°
	const f32 base_rotation = -PI / 2; // 90°
	const f32 left_angle = -cone_angle_padding + base_rotation + angle;
	const f32 right_angle = cone_angle_padding + base_rotation + angle;

	Vector2 p1 = *(Vector2*)&pos;
	f32 ray_len = 15.0;

	Vector2 p_left = {
		.x = cos(left_angle) * ray_len + p1.x,
		.y = sin(left_angle) * ray_len + p1.y,
	};
	Vector2 p_right = {
		.x = cos(right_angle) * ray_len + p1.x,
		.y = sin(right_angle) * ray_len + p1.y,
	};

	const u32 player_size = BLOCK_SIZE / 8;
	DrawCircleV(pos, player_size, GREEN);
	DrawLineV(p1, p_left, WHITE);
	DrawLineV(p1, p_right, WHITE);
	DrawLineV(p_left, p_right, WHITE);
}

void update_player(scene_t *scene, f32 delta_time)
{
	vec2f32_t speed = { 0, 0 };

	if (IsKeyDown(KEY_W)) {
		speed.y -= PLAYER_SPEED;
	}
	if (IsKeyDown(KEY_S)) {
		speed.y += PLAYER_SPEED;
	}
	if (IsKeyDown(KEY_A)) {
		speed.x -= PLAYER_SPEED;
	}
	if (IsKeyDown(KEY_D)) {
		speed.x += PLAYER_SPEED;
	}

	const f32 max_x = scene->width * BLOCK_SIZE;
	const f32 new_x = scene->player_position.x + speed.x * delta_time;
	scene->player_position.x = MAX(MIN(new_x, max_x), 0);

	const f32 max_y = scene->height * BLOCK_SIZE;
	const f32 new_y = (scene->player_position.y + speed.y * delta_time);
	scene->player_position.y = MAX(MIN(new_y, max_y), 0);

	if (IsKeyDown(KEY_LEFT)) {
		scene->player_angle -= 1.0f * delta_time;
	}

	if (IsKeyDown(KEY_RIGHT)) {
		scene->player_angle += 1.0f * delta_time;
	}
}

int main(void)
{
	InitWindow(SCREEN_WITDH, SCREEN_HEIGHT, "RayCast");
	const i32 color = 0xff00ffff;

	block_e blocks[9*9] = {BLOCK_EMPTY};
	scene_t scene = {
		.width = 9,
		.height = 9,
		.blocks = blocks,
		.player_position = {.x = 3, .y = 3 },
	};
	scene.blocks[0] = BLOCK_BRICKS;
	scene.blocks[2] = BLOCK_BRICKS;
	scene.blocks[4] = BLOCK_BRICKS;
	scene.blocks[6] = BLOCK_BRICKS;
	scene.blocks[8] = BLOCK_BRICKS;
	// for (u32 i = 0; i < scene.width * scene.height; i ++) {
	// 	scene.blocks[i] = BLOCK_BRICKS;
	// }

	RenderTexture2D minimap = LoadRenderTexture(scene.width * BLOCK_SIZE, scene.height * BLOCK_SIZE);

	vec2f32_t speed = { 1.0f, 1.0f };
	SetTargetFPS(60);
	while(!WindowShouldClose()) {
		BeginTextureMode(minimap);
			ClearBackground(BLACK);
			draw_grid(scene.blocks, scene.width, scene.height);
			draw_player_view(*(Vector2*)&scene.player_position, scene.player_angle);
		EndTextureMode();

		BeginDrawing();
			ClearBackground(BLACK);

			DrawTexturePro(
					minimap.texture,
					(Rectangle){.x = 0, .y = 0, .width = minimap.texture.width, .height = -minimap.texture.height},
					// (Rectangle){.x = 0, .y = 0, .width = minimap.texture.width, .height = -minimap.texture.height},
					(Rectangle){.x = 2, .y = 2, .width = 128, .height = 128},
					(Vector2) {0, 0},
					0,
					WHITE);
			DrawRectangleLines(0, 0, 131, 131, GOLD);
		EndDrawing();

		f32 delta_time = GetFrameTime();
		update_player(&scene, delta_time);
	}

	printf("Hello, world!\n");
	return 0;
}
