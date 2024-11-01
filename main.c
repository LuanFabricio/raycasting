#include <stdio.h>
#include <math.h>
#include <float.h>
#include "raylib.h"

#include "src/types.h"
#include "src/utils.h"
#include "src/vec2f32.h"

#define BLOCK_SIZE 16

#define SCALE 80

#define SCREEN_WITDH 16*SCALE
#define SCREEN_HEIGHT 9*SCALE

#define PLAYER_SPEED 100

#define MIN(x, y) (x) > (y) ? (y) : (x)
#define MAX(x, y) (x) < (y) ? (y) : (x)

#define BASE_ROTATION (-PI / 2)

#define FAR_DISTANCE 45.0
#define NEAR_DISTANCE 15.0
#define FOV (-PI / 4) // -45°

void get_fov_plane(const vec2f32_t pos, const f32 angle, vec2f32_t out[2])
{
	vec2f32_t plane_middle = vec2f32_from_angle(BASE_ROTATION + angle);
	vec2f32_scale(&plane_middle, NEAR_DISTANCE, &plane_middle);
	vec2f32_add(&plane_middle , &CAST_TYPE(vec2f32_t, pos), &plane_middle);

	f32 left = tan(FOV * 0.5) * NEAR_DISTANCE;

	vec2f32_sub(&plane_middle, &CAST_TYPE(vec2f32_t, pos), &out[0]);
	vec2f32_rot90(&out[0], &out[0]);
	vec2f32_norm(&out[0], &out[0]);
	vec2f32_scale(&out[0], left, &out[0]);
	vec2f32_add(&plane_middle, &out[0], &out[0]);

	vec2f32_sub(&plane_middle, &CAST_TYPE(vec2f32_t, pos), &out[1]);
	vec2f32_rot90(&out[1], &out[1]);
	vec2f32_norm(&out[1], &out[1]);
	vec2f32_scale(&out[1], left, &out[1]);
	vec2f32_sub(&plane_middle, &out[1], &out[1]);
}

f32 point_distance(vec2f32_t p1, vec2f32_t p2)
{
	f32 dx = p1.x - p2.x;
	f32 dy = p1.y - p2.y;

	return sqrt(dx*dx + dy*dy);
}

bool get_intersection(vec2f32_t s1, vec2f32_t e1, vec2f32_t s2, vec2f32_t e2, vec2f32_t *out)
{
	f32 dx = e1.x - s1.x;
	f32 dy = e1.y - s1.y;

	bool vertical1 = dx == 0.0;
	f32 a1 = vertical1 ? 0.0 : dy / dx;
	f32 b1 = e1.y - a1 * e1.x;

	dx = e2.x - s2.x;
	dy = e2.y - s2.y;

	bool vertical2 = dx == 0.0;
	f32 a2 = vertical2 ? 0.0 : dy / dx;
	f32 b2 = e2.y - a2 * e2.x;

	if (a1 == a2 && vertical1 == vertical2) return false;
	if (vertical1 && vertical2) return false;

	if (vertical1) {
		out->x = s1.x;
		out->y = a2*out->x + b2;
	} else if (vertical2) {
		out->x = s2.x;
		out->y = a1*out->x + b1;
	} else {
		out->x = (b2 - b1) / (a1 - a2);
		out->y = a1*out->x + b1;
	}

	// Line 1
	f32 min_x = s1.x < e1.x ? s1.x : e1.x;
	f32 max_x = s1.x > e1.x ? s1.x : e1.x;

	f32 min_y = s1.y < e1.y ? s1.y : e1.y;
	f32 max_y = s1.y > e1.y ? s1.y : e1.y;

	if (min_x > out->x || out->x > max_x) return false;
	if (min_y > out->y || out->y > max_y) return false;

	// Line 2
	min_x = s2.x < e2.x ? s2.x : e2.x;
	max_x = s2.x > e2.x ? s2.x : e2.x;

	min_y = s2.y < e2.y ? s2.y : e2.y;
	max_y = s2.y > e2.y ? s2.y : e2.y;

	if (min_x > out->x || out->x > max_x) return false;
	if (min_y > out->y || out->y > max_y) return false;

	return true;
}

void get_block_points(u32 x, u32 y, vec2f32_t *out)
{
	// (x, y)
	out[0].x = x * BLOCK_SIZE;
	out[0].y = y * BLOCK_SIZE;
	// (x+1, y)
	out[1].x = (x+1) * BLOCK_SIZE;
	out[1].y = y * BLOCK_SIZE;
	// (x+1, y+1)
	out[2].x = (x+1) * BLOCK_SIZE;
	out[2].y = (y+1) * BLOCK_SIZE;
	// (x, y+1)
	out[3].x = x * BLOCK_SIZE;
	out[3].y = (y+1) * BLOCK_SIZE;
}

i32 point_in_block(const block_e* blocks, const u32 blocks_len, const u32 width, vec2f32_t point)
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
				return i;
			}
		}
	}

	return -1;
}

bool hit_block(const scene_t *scene, const vec2f32_t p1, const vec2f32_t p2, vec2f32_t *hit)
{
	vec2f32_t points[4] = {0};
	const u32 blocks_len = scene->width * scene->height;

	const u32 lines[] = { 0, 1, 1, 2, 2, 3, 3, 0};
	bool have_hit = false;

	f32 dist = FLT_MAX;
	vec2f32_t current_hit = {};
	for (u32 i = 0; i < blocks_len; i++) {
		if (scene->blocks[i] != BLOCK_BRICKS) continue;
		vec2u32_t block_pos = index_to_xy(i, scene->width);
		get_block_points(block_pos.x, block_pos.y, points);
		for (u32 j = 0; j < sizeof(lines)/sizeof(u32); j+=2) {
			printf("[%u, %u->%u]:\n", i, j, j+1);
			printf("\t(%.02f, %.02f) -> (%.02f, %.02f)\n", p1.x, p1.y, p2.x, p2.y);
			printf("\t(%.02f, %.02f) -> (%.02f, %.02f)\n", points[lines[j]].x, points[lines[j]].y, points[lines[j+1]].x, points[lines[j+1]].y);
			if (get_intersection(p1, p2, points[lines[j]], points[lines[j+1]], &current_hit)) {
				f32 current_dist = point_distance(p1, current_hit);
				if (current_dist < dist) {
					dist = current_dist;
					*hit = current_hit;
				}

				have_hit = true;
			}
		}
	}

	return have_hit;
}

void render_scene(const scene_t *scene)
{
	const f32 base_rotation = -PI / 2; // 90°

	Vector2 p1 = *(Vector2*)&scene->player_position;

	const f32 angle = scene->player_angle;
	const f32 left_angle = FOV/2 + base_rotation + angle;
	const f32 right_angle = -FOV/2 + base_rotation + angle;

	const u32 rays = 5;
	double angle_step =  2 * FOV/2 / (rays-1);
	printf("Delta angle: %.02f\n", left_angle - right_angle);
	printf("Angle step: %.02f\n", angle_step);

	Vector2 p2 = {};
	Vector2 p3 = {};

	const u32 block_len = scene->width*scene->height;
	const u32 width_slice = GetScreenWidth() / rays;
	const u32 height_slice = GetScreenHeight() / rays;

	const f32 near_clipping_plane = 0.5;
	vec2f32_t plane_middle = {
		.x = cos(base_rotation + angle) * near_clipping_plane,
		.y = sin(base_rotation + angle) * near_clipping_plane,
	};

	f32 left = tan(FOV/2) * near_clipping_plane;
	vec2f32_t pl = {};

	vec2f32_add(&plane_middle, &scene->player_position, &pl);
	vec2f32_rot90(&pl, &pl);
	vec2f32_norm(&pl, &pl);
	vec2f32_norm(&pl, &pl);
	vec2f32_scale(&pl, left, &pl);

	const u32 screen_width = 10;
	const u32 strip_width = (float)GetScreenWidth() / (float)screen_width;

	vec2f32_t fov_plane[2] = {0};
	get_fov_plane(scene->player_position, scene->player_angle, fov_plane);

	for (u32 x = 0; x < screen_width; x++) {
		vec2f32_t ray = {0};
		const f32 amount = (float)x / (float)screen_width;
		printf("Lerp: \n");
		printf("\tFov left: (%.02f, %.02f)\n", fov_plane[0].x, fov_plane[0].y);
		printf("\tFov right: (%.02f, %.02f)\n", fov_plane[1].x, fov_plane[1].y);
		printf("\tAmount: %.02f\n", amount);
		vec2f32_lerp(&fov_plane[0], &fov_plane[1], amount, &ray);
		printf("\tRay: (%.02f, %.02f)\n", ray.x, ray.y);

		vec2f32_t hit = {0};
		const bool res = hit_block(scene, scene->player_position, ray, &hit);
		printf("[Hit? %b] (%.02f, %.02f) -> (%.02f, %.02f)\n", res, ray.x, ray.y, hit.x, hit.y);
		if (res) {
			DrawRectangle(x*strip_width, 0, strip_width, GetScreenHeight(), RED);
		}
	}
}

void draw_step_function(const scene_t *scene)
{
	const f32 base_rotation = -PI / 2; // 90°
	const f32 final_angle = base_rotation + scene->player_angle;
	const f32 x_rotation = cos(final_angle);
	const f32 y_rotation = sin(final_angle);

	const u32 steps = 5;
	const f32 ray_step = FAR_DISTANCE / steps;

	Vector2 p1 = *(Vector2*)&scene->player_position;
	Vector2 p2 = {};

	const u32 blocks_len = scene->width * scene->height;
	Color color = WHITE;
	for (u32 i = 1; i <= steps; i++) {
		p2.x = x_rotation * ray_step + p1.x;
		p2.y = y_rotation * ray_step + p1.y;

		i32 block_index = point_in_block(scene->blocks, blocks_len, scene->width, CAST_TYPE(vec2f32_t, p2));

		bool res = false;
		vec2f32_t intersection_point = {0};

		if (block_index != -1) {
			color = PURPLE;

			vec2u32_t point = index_to_xy(block_index, scene->width);
			vec2f32_t block_points[4] = {0};
			get_block_points(point.x, point.y, block_points);

			u32 lines[] = { 0, 1, 1, 2, 2, 3, 3, 0};
			for (u32 i = 0; i < 8; i+=2) {
				res = get_intersection(
					CAST_TYPE(vec2f32_t, p1), CAST_TYPE(vec2f32_t, p2),
					block_points[lines[i]], block_points[lines[i+1]],
					&intersection_point);

				if (res) break;
			}
		}

		DrawLineV(p1, p2, color);
		DrawCircleV(p2, 2, color);
		if(res) {
			DrawCircleV(CAST_TYPE(Vector2, intersection_point), 1, GREEN);
		}

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
	const f32 base_rotation = -PI / 2; // 90°

	Vector2 p1 = *(Vector2*)&pos;

	// const f32 left_angle = FOV/2 + base_rotation + angle;
	// const f32 right_angle = -FOV/2 + base_rotation + angle;

	// const u32 rays = 0;
	// double angle_step =  (left_angle - right_angle) / (rays-1);

	// Vector2 p2 = {};
	// for (i32 i = 0; i < rays; i++) {
	// 	p2.x = cos(left_angle - angle_step * i) * FAR_DISTANCE + p1.x;
	// 	p2.y = sin(left_angle - angle_step * i) * FAR_DISTANCE + p1.y;
	// 	DrawLineV(p1, p2, (i % 2) == 0 ? RED : BLUE);
	// }

	const u32 player_size = 3;
	DrawCircleV(pos, player_size, GREEN);

	// p2.x = cos(left_angle) * FAR_DISTANCE + p1.x;
	// p2.y = sin(left_angle) * FAR_DISTANCE + p1.y;

	// Vector2 p3 = {
	// 	.x = cos(right_angle) * FAR_DISTANCE + p1.x,
	// 	.y = sin(right_angle) * FAR_DISTANCE + p1.y,
	// };

	vec2f32_t plane_middle = vec2f32_from_angle(base_rotation + angle);
	vec2f32_scale(&plane_middle, NEAR_DISTANCE, &plane_middle);
	vec2f32_add(&plane_middle , &CAST_TYPE(vec2f32_t, pos), &plane_middle);

	vec2f32_t pov_plane[2] = {0};
	get_fov_plane(CAST_TYPE(vec2f32_t, pos), angle, pov_plane);

	DrawLineV(CAST_TYPE(Vector2, plane_middle), CAST_TYPE(Vector2, pov_plane[0]), WHITE);
	DrawLineV(pos, CAST_TYPE(Vector2, pov_plane[0]), WHITE);

	DrawLineV(CAST_TYPE(Vector2, plane_middle), CAST_TYPE(Vector2, pov_plane[1]), WHITE);
	DrawLineV(pos, CAST_TYPE(Vector2, pov_plane[1]), WHITE);


	DrawCircleV(CAST_TYPE(Vector2, pov_plane[0]), 1, BLUE);
	DrawCircleV(CAST_TYPE(Vector2, plane_middle), 1, RED);
	DrawCircleV(CAST_TYPE(Vector2, pov_plane[1]), 1, BLUE);
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
	// scene.blocks[2] = BLOCK_BRICKS;
	// scene.blocks[4] = BLOCK_BRICKS;
	// scene.blocks[6] = BLOCK_BRICKS;
	// scene.blocks[8] = BLOCK_BRICKS;

	// for (u32 i = 0; i < scene.width * scene.height; i ++) {
	// 	scene.blocks[i] = BLOCK_BRICKS;
	// }

	RenderTexture2D minimap = LoadRenderTexture(scene.width * BLOCK_SIZE, scene.height * BLOCK_SIZE);

	vec2f32_t speed = { 1.0f, 1.0f };
	const u32 minimap_size = 320;

	SetTargetFPS(60);
	while(!WindowShouldClose()) {
		BeginTextureMode(minimap);
			ClearBackground(BLACK);
			draw_grid(scene.blocks, scene.width, scene.height);
			//draw_step_function(&scene);
			draw_player_view(*(Vector2*)&scene.player_position, scene.player_angle);
		EndTextureMode();

		BeginDrawing();
			ClearBackground(BLACK);

			render_scene(&scene);

			DrawTexturePro(minimap.texture,
					(Rectangle) {.x = 0, .y = 0, .width = minimap.texture.width, .height = -minimap.texture.height},
					(Rectangle) {.x = 2, .y = 2, .width = minimap_size, .height = minimap_size},
					// (Rectangle) {.x = 2, .y = 2, .width = GetScreenWidth(), .height = GetScreenHeight()},
					(Vector2) {0, 0},
					0,
					WHITE);
			DrawRectangleLines(0, 0, minimap_size+4, minimap_size+4, GOLD);
		EndDrawing();

		f32 delta_time = GetFrameTime();
		update_player(&scene, delta_time);
	}

	return 0;
}
