#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include "raylib.h"

#include "src/types.h"
#include "src/utils.h"
#include "src/vec2f32.h"

#include "src/defines.h"

#include "src/render.h"

bool get_intersection(vec2f32_t s1, vec2f32_t e1, vec2f32_t s2, vec2f32_t e2, vec2f32_t *out)
{
	f32 dx = e1.x - s1.x;
	f32 dy = e1.y - s1.y;

	bool vertical1 = dx == 0.0;
	f32 a1 = vertical1 ? 0.0 : dy / dx;
	f32 b1 = e1.y - a1 * e1.x;

#ifdef LOG
	printf("P1:\n");
	printf("\tdx: %.02f\n", dx);
	printf("\tdy: %.02f\n", dy);
	printf("\talpha: %.02f\n", a1);
	printf("\tlin: %.02f\n", b1);
	printf("\tvertical: %b\n", vertical1);
#endif // LOG

	dx = e2.x - s2.x;
	dy = e2.y - s2.y;

	bool vertical2 = dx == 0.0;
	f32 a2 = vertical2 ? 0.0 : dy / dx;
	f32 b2 = e2.y - a2 * e2.x;

#ifdef LOG
	printf("P2:\n");
	printf("\tdx: %.02f\n", dx);
	printf("\tdy: %.02f\n", dy);
	printf("\talpha: %.02f\n", a2);
	printf("\tlin: %.02f\n", b2);
	printf("\tvertical: %b\n", vertical2);
#endif // LOG

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

#ifdef LOG
	printf("Out: %.05f, %.05f\n", out->x, out->y);
#endif // LOG

	// Line 1
	f32 min_x = s1.x < e1.x ? s1.x : e1.x;
	f32 max_x = s1.x > e1.x ? s1.x : e1.x;

	f32 min_y = s1.y < e1.y ? s1.y : e1.y;
	f32 max_y = s1.y > e1.y ? s1.y : e1.y;

#ifdef LOG
	printf("Out: %.05f, %.05f\n", out->x, out->y);
	printf("P1 min/max x: %.05f, %.05f\n", min_x, max_x);
	printf("P1 min/max y: %.05f, %.05f\n", min_y, max_y);
#endif // LOG

	if ((min_x - out->x) > EPSILON || (out->x - max_x) > EPSILON) return false;
	if ((min_y - out->y) > EPSILON || (out->y - max_y) > EPSILON) return false;
	// Line 2
	min_x = s2.x < e2.x ? s2.x : e2.x;
	max_x = s2.x > e2.x ? s2.x : e2.x;

	min_y = s2.y < e2.y ? s2.y : e2.y;
	max_y = s2.y > e2.y ? s2.y : e2.y;

#ifdef LOG
	printf("P2 min/max x: %.05f, %.05f\n", min_x, max_x);
	printf("P2 min/max y: %.05f, %.05f\n", min_y, max_y);
#endif // LOG

	if ((min_x - out->x) > EPSILON || (out->x - max_x) > EPSILON) return false;
	if ((min_y - out->y) > EPSILON || (out->y - max_y) > EPSILON) return false;

	return true;
}

void get_block_points(u32 x, u32 y, float scale, vec2f32_t *out)
{
	// (x, y)
	out[0].x = x 	 * scale;
	out[0].y = y 	 * scale;
	// (x+1, y)
	out[1].x = (x+1) * scale;
	out[1].y = y 	 * scale;
	// (x+1, y+1)
	out[2].x = (x+1) * scale;
	out[2].y = (y+1) * scale;
	// (x, y+1)
	out[3].x = x	 * scale;
	out[3].y = (y+1) * scale;
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
		get_block_points(block_pos.x, block_pos.y, 1.0f, points);
		for (u32 j = 0; j < sizeof(lines)/sizeof(u32); j+=2) {
#ifdef LOG
			printf("[%u, %u->%u]:\n", i, j, j+1);
			printf("\t(%.02f, %.02f) -> (%.02f, %.02f)\n", p1.x, p1.y, p2.x, p2.y);
			printf("\t(%.02f, %.02f) -> (%.02f, %.02f)\n", points[lines[j]].x, points[lines[j]].y, points[lines[j+1]].x, points[lines[j+1]].y);
#endif // LOG
			if (get_intersection(p1, p2, points[lines[j]], points[lines[j+1]], &current_hit)) {
				f32 current_dist = vec2f32_distance(&p1, &current_hit);
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
	const u32 screen_width = 512;
	const u32 strip_width = 1 + GetScreenWidth() / screen_width;

	vec2f32_t fov_plane[2] = {0};
	get_fov_plane(scene->player_position, scene->player_angle, FAR_DISTANCE, fov_plane);

	vec2f32_t player_ray = vec2f32_from_angle(scene->player_angle + BASE_ROTATION);
	vec2f32_scale(&player_ray, 1/vec2f32_length(&player_ray), &player_ray);
	for (u32 x = 0; x < screen_width; x++) {
		vec2f32_t ray = {0};
		const f32 amount = (float)x / (float)screen_width;
		vec2f32_lerp(&fov_plane[0], &fov_plane[1], amount, &ray);
		vec2f32_t hit = {0};
		const bool res = hit_block(scene, scene->player_position, ray, &hit);

#ifdef LOG
		printf("Lerp: \n");
		printf("\tFov left: (%.02f, %.02f)\n", fov_plane[0].x, fov_plane[0].y);
		printf("\tFov right: (%.02f, %.02f)\n", fov_plane[1].x, fov_plane[1].y);
		printf("\tAmount: %.02f\n", amount);
		printf("\tRay: (%.02f, %.02f)\n", ray.x, ray.y);
		printf("[%u - Hit? %b] (%.02f, %.02f) -> (%.02f, %.02f)\n", x, res, ray.x, ray.y, hit.x, hit.y);
#endif // LOG

		if (res) {
			vec2f32_t v = {0};
			// printf("Hit: %.02f %.02f\n", hit.x, hit.y);
			// printf("Player pos: %.02f %.02f\n", scene->player_position.x, scene->player_position.y);
			vec2f32_sub(&hit, &scene->player_position, &v);
			// printf("V: %.02f, %.02f\n", v.x, v.y);
			// printf("Player ray: %.02f, %.02f\n", player_ray.x, player_ray.y);
			const f32 z = vec2f32_dot(&v, &player_ray);
			const f32 strip_height = (f32)GetScreenHeight() / z;
			const u32 y = (GetScreenHeight() - strip_height) / 2;
			// printf("[%u]Height strip: %.02f/%.02f\n", x, strip_height, z);
			Color color = {.r = 0xff, .g = 0x10, .b = 0x10, .a = 0xff};
			const f32 shadow = MIN(1.0f/z*4.0f, 1.0f);

			color.r *= shadow;
			color.g *= shadow;
			color.b *= shadow;
			DrawRectangle(x*strip_width, y, strip_width, strip_height, color);
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
			get_block_points(point.x, point.y, BLOCK_SIZE, block_points);

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

void draw_player_view(Vector2 pos, f32 angle)
{
	pos.x *= BLOCK_SIZE;
	pos.y *= BLOCK_SIZE;
	Vector2 p1 = *(Vector2*)&pos;

	const f32 fov_length = FAR_DISTANCE*BLOCK_SIZE;

	vec2f32_t plane_middle = vec2f32_from_angle(BASE_ROTATION + angle);
	vec2f32_scale(&plane_middle, fov_length, &plane_middle);
	vec2f32_add(&plane_middle , &CAST_TYPE(vec2f32_t, pos), &plane_middle);

	vec2f32_t pov_plane[2] = {0};
	get_fov_plane(CAST_TYPE(vec2f32_t, pos), angle, fov_length, pov_plane);

	DrawLineV(CAST_TYPE(Vector2, plane_middle), CAST_TYPE(Vector2, pov_plane[0]), WHITE);
	DrawLineV(pos, CAST_TYPE(Vector2, pov_plane[0]), WHITE);

	DrawLineV(CAST_TYPE(Vector2, plane_middle), CAST_TYPE(Vector2, pov_plane[1]), WHITE);
	DrawLineV(pos, CAST_TYPE(Vector2, pov_plane[1]), WHITE);


	// DrawCircleV(CAST_TYPE(Vector2, pov_plane[0]), 1, BLUE);
	// DrawCircleV(CAST_TYPE(Vector2, plane_middle), 1, RED);
	// DrawCircleV(CAST_TYPE(Vector2, pov_plane[1]), 1, BLUE);

	DrawCircleV(pos, 1, GREEN);
}

void update_player(scene_t *scene, f32 delta_time)
{
	vec2f32_t speed = { 0, 0 };
	const vec2f32_t direction = vec2f32_from_angle(scene->player_angle + BASE_ROTATION);

	if (IsKeyDown(KEY_W)) {
		speed.x += PLAYER_SPEED * direction.x;
		speed.y += PLAYER_SPEED * direction.y;
	}
	if (IsKeyDown(KEY_S)) {
		speed.x -= PLAYER_SPEED * direction.x;
		speed.y -= PLAYER_SPEED * direction.y;
	}
	if (IsKeyDown(KEY_A)) {
		speed.x -= PLAYER_SPEED * -direction.y;
		speed.y -= PLAYER_SPEED * direction.x;
	}
	if (IsKeyDown(KEY_D)) {
		speed.x += PLAYER_SPEED * -direction.y;
		speed.y += PLAYER_SPEED * direction.x;
	}

	const f32 max_x = scene->width; //* BLOCK_SIZE;
	const f32 new_x = scene->player_position.x + speed.x * delta_time;
	scene->player_position.x = MAX(MIN(new_x, max_x), 0);

	const f32 max_y = scene->height; // * BLOCK_SIZE;
	const f32 new_y = (scene->player_position.y + speed.y * delta_time);
	scene->player_position.y = MAX(MIN(new_y, max_y), 0);

	if (IsKeyDown(KEY_LEFT)) {
		scene->player_angle -= 0.75f * delta_time;
	}

	if (IsKeyDown(KEY_RIGHT)) {
		scene->player_angle += 0.75f * delta_time; }
}

int main(void)
{
	InitWindow(SCREEN_WITDH, SCREEN_HEIGHT, "RayCast");
	const i32 color = 0xff00ffff;

	block_e blocks[SCENE_WIDTH*SCENE_HEIGHT] = {BLOCK_EMPTY};
	scene_t scene = {
		.width = SCENE_WIDTH,
		.height = SCENE_HEIGHT,
		.blocks = blocks,
		.player_angle = -PI / 4,
	};
	scene.player_position.x = scene.width / 2.0f;// * BLOCK_SIZE / 2.f;
	scene.player_position.y = scene.height / 2.0f;// * BLOCK_SIZE / 2.f;

	scene.blocks[0] = BLOCK_BRICKS;
	scene.blocks[xy_to_index(3, 3, scene.width)] = BLOCK_BRICKS;
	scene.blocks[xy_to_index(2, 3, scene.width)] = BLOCK_BRICKS;
	scene.blocks[xy_to_index(3, 2, scene.width)] = BLOCK_BRICKS;
	// scene.blocks[2] = BLOCK_BRICKS;
	// scene.blocks[4] = BLOCK_BRICKS;
	// scene.blocks[6] = BLOCK_BRICKS;
	// scene.blocks[8] = BLOCK_BRICKS;
	// scene.blocks[10] = BLOCK_BRICKS;
	// scene.blocks[12] = BLOCK_BRICKS;
	// scene.blocks[14] = BLOCK_BRICKS;
	// scene.blocks[16] = BLOCK_BRICKS;
	// scene.blocks[18] = BLOCK_BRICKS;

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
