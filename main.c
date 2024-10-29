#include <stdio.h>
#include <math.h>
#include "raylib.h"

#include "src/types.h"
#include "src/utils.h"

#define BLOCK_SIZE 32

#define SCALE 80

#define SCREEN_WITDH 16*SCALE
#define SCREEN_HEIGHT 9*SCALE

#define PLAYER_SPEED 100

#define MIN(x, y) (x) > (y) ? (y) : (x)
#define MAX(x, y) (x) < (y) ? (y) : (x)

#define FAR_DISTANCE 10

bool is_inside_a_box(const block_e* blocks, const u32 blocks_len, const u32 width, vec2f32_t point)
{
	for (u32 i = 0; i < blocks_len; i++) {
		block_e block = blocks[i];

		if (block == BLOCK_BRICKS) {
			vec2u32_t min_point = index_to_xy(i, width);
			min_point.x *= BLOCK_SIZE;
			min_point.y *= BLOCK_SIZE;

			vec2u32_t max_point = {
				.x = min_point.x + BLOCK_SIZE,
				.y = min_point.y + BLOCK_SIZE,
			};

			const bool x_match = min_point.x <= point.x && point.x <= max_point.x;
			const bool y_match = min_point.y <= point.y && point.y <= max_point.y;

			if (x_match && y_match) {
				return true;
			}
		}
	}

	return false;
}

void draw_step_function(const scene_t *scene)
{
	const f32 base_rotation = -PI / 2; // 90°
	const f32 final_angle = base_rotation + scene->player_angle;
	const f32 x_rotation = cos(final_angle);
	const f32 y_rotation = sin(final_angle);

	const f32 ray_len = 45.0;
	const u32 steps = 5;
	const f32 ray_step = ray_len / steps;

	Vector2 p1 = *(Vector2*)&scene->player_position;
	Vector2 p2 = {};

	const u32 blocks_len = scene->width * scene->height;
	Color color = WHITE;
	for (u32 i = 1; i <= steps; i++) {
		p2.x = x_rotation * ray_step + p1.x;
		p2.y = y_rotation * ray_step + p1.y;

		// TODO: Maybe draw an intesection point
		if (is_inside_a_box(scene->blocks, blocks_len, scene->width, CAST_TYPE(vec2f32_t, p2))) {
			color = PURPLE;
		}

		DrawLineV(p1, p2, color);
		DrawCircleV(p2, 2, color);

		p1.x = p2.x;
		p1.y = p2.y;
	}
}

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

	Vector2 p1 = *(Vector2*)&pos;
	f32 ray_len = 45.0;

	const f32 left_angle = cone_angle_padding + base_rotation + angle;
	const f32 right_angle = -cone_angle_padding + base_rotation + angle;

	const u32 rays = 4;
	double angle_step =  (left_angle - right_angle) / (rays-1);

	Vector2 p2 = {};
	for (i32 i = 0; i < rays; i++) {
		p2.x = cos(left_angle - angle_step * i) * ray_len + p1.x;
		p2.y = sin(left_angle - angle_step * i) * ray_len + p1.y;
		DrawLineV(p1, p2, (i % 2) == 0 ? RED : BLUE);
	}

	const u32 player_size = 3;
	DrawCircleV(pos, player_size, GREEN);

	p2.x = cos(left_angle) * ray_len + p1.x;
	p2.y = sin(left_angle) * ray_len + p1.y;

	Vector2 p3 = {
		.x = cos(right_angle) * ray_len + p1.x,
		.y = sin(right_angle) * ray_len + p1.y,
	};
	DrawLineV(p2, p3, WHITE);
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
		.player_angle = 0,
	};
	scene.player_position.x = scene.width * BLOCK_SIZE / 2.f;
	scene.player_position.y = scene.height * BLOCK_SIZE / 2.f;

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
			draw_step_function(&scene);
		EndTextureMode();

		BeginDrawing();
			ClearBackground(BLACK);

			DrawTexturePro(minimap.texture,
					(Rectangle) {.x = 0, .y = 0, .width = minimap.texture.width, .height = -minimap.texture.height},
					// (Rectangle){.x = 0, .y = 0, .width = minimap.texture.width, .height = -minimap.texture.height},
					(Rectangle) {.x = 2, .y = 2, .width = GetScreenWidth(), .height = GetScreenHeight()},
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
