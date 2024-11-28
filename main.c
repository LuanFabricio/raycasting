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
#include "src/scene.h"
#include "src/collision.h"

void render_texture(
		const render_texture_t *tex_data,
		const u32 screen_x,
		const u32 screen_height,
		const f32 shadow
)
{
	const f32 text_height_prop = tex_data->size.y / tex_data->strip.y;

	const i32 y_start = MAX(0, tex_data->coords.y);
	const i32 y_end = MIN(screen_height, tex_data->coords.y + tex_data->strip.y);

	for (i32 y = y_start; y < y_end; y++) {
		const u32 tex_y = render_get_texture_y(y, tex_data->coords.y, text_height_prop);
		const u32 tex_point = render_get_texture_color_index(
				tex_data->coords.x, tex_y, tex_data->size.y);

		const u32 color_u32 = color_apply_shadow(tex_data->pixels[tex_point], shadow);
		Color color = CAST_TYPE(Color, color_u32);

		DrawRectangle(screen_x, y, tex_data->strip.x, 1, color);
	}
}

void render_scene(const scene_t *scene)
{
	// TODO: Check the coord types, maybe move from u32 to i32.
	const u32 strip_width = 1 + GetScreenWidth() / RENDER_WIDTH;
	const u32 strip_height = 1 + GetScreenHeight() / RENDER_HEIGHT;

	vec2f32_t fov_plane[2] = {0};
	get_fov_plane(scene->player_position, scene->player_angle, FAR_DISTANCE, fov_plane);

	vec2f32_t player_ray = vec2f32_from_angle(scene->player_angle);
	vec2f32_scale(&player_ray, 1/vec2f32_length(&player_ray), &player_ray);

	u32 screen_height = GetScreenHeight();
	for (u32 x = 0; x < RENDER_WIDTH; x++) {
		vec2f32_t ray = {0};
		const f32 amount = (float)x / (float)RENDER_WIDTH;
		vec2f32_lerp(&fov_plane[0], &fov_plane[1], amount, &ray);

		vec2f32_t hit = {0};
		block_t *block = NULL;
		u8 block_face = 0xff;
		const bool res = collision_hit_a_block(scene, scene->player_position, ray, &hit, &block, &block_face);

#ifdef LOG
		printf("Lerp: \n");
		printf("\tFov left: (%.02f, %.02f)\n", fov_plane[0].x, fov_plane[0].y);
		printf("\tFov right: (%.02f, %.02f)\n", fov_plane[1].x, fov_plane[1].y);
		printf("\tAmount: %.02f\n", amount);
		printf("\tRay: (%.02f, %.02f)\n", ray.x, ray.y);
		printf("[%u - Hit? %b] (%.02f, %.02f) -> (%.02f, %.02f)\n", x, res, ray.x, ray.y, hit.x, hit.y);
#endif // LOG

		if (!res) continue;

		vec2f32_t v = {0};
		vec2f32_sub(&hit, &scene->player_position, &v);
		const f32 perp_wall_dist = vec2f32_dot(&v, &player_ray);
		const f32 strip_height = (f32)screen_height / perp_wall_dist;
		const i32 y = (screen_height - strip_height) * 0.5f;

		const f32 shadow = MIN(1.0f/perp_wall_dist*4.0f, 1.0f);

		// TODO: Move render.h
		switch (block->block_type) {
			case BLOCK_COLOR: {
				u32 color_u32 = *(u32*)block->data;
				color_u32 = color_apply_shadow(color_u32, shadow);
				Color color = CAST_TYPE(Color, color_u32);

				DrawRectangle(x*strip_width, y, strip_width, strip_height, color);
			} break;
			case BLOCK_BRICKS: {
				const u32 src_x = render_get_texture_x(&hit, block_face);
				const render_texture_t tex_data = {
					.pixels = block->data,
					.coords = { src_x, y },
					.strip = { strip_width, strip_height },
					.size = { TEXTURE_SIZE, TEXTURE_SIZE },
				};
				render_texture(
					&tex_data,
					x*strip_width, screen_height, shadow);
			} break;

			default:
				break;
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

		i32 block_index = collision_point_in_block(scene->blocks, blocks_len, scene->width, CAST_TYPE(vec2f32_t, p2));

		bool res = false;
		vec2f32_t intersection_point = {0};

		if (block_index != -1) {
			color = PURPLE;

			vec2u32_t point = index_to_xy(block_index, scene->width);
			vec2f32_t block_points[4] = {0};
			scene_get_block_points(point.x, point.y, BLOCK_SIZE, block_points);

			u32 lines[] = { 0, 1, 1, 2, 2, 3, 3, 0};
			for (u32 i = 0; i < 8; i+=2) {
				res = collision_intersects(
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

void draw_grid(const block_t *blocks, u32 width, u32 height)
{
	for (u32 yy = 0; yy < height; yy++) {
		for (u32 xx = 0; xx < width; xx++) {
			const u32 index = xy_to_index(xx, yy, width);

			block_t block = blocks[index];
			block_type_e block_type = block.block_type;
			switch (block_type) {
				case BLOCK_COLOR: {
					u32 color = *(u32*)block.data;
					DrawRectangle(xx*BLOCK_SIZE, yy*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, CAST_TYPE(Color, color));
				} break;

				case BLOCK_BRICKS: {
					// TODO: Render a texture instead of color
					DrawRectangle(xx*BLOCK_SIZE, yy*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, PURPLE);
					// Image img = LoadImage("./assets/textures/bricksx64.png");
					// ImageResize(&img, BLOCK_SIZE, BLOCK_SIZE);
					// Texture2D tex = LoadTextureFromImage(img);
					// UpdateTexture(tex, block.data);
					// DrawTexture(tex, xx*BLOCK_SIZE, yy*BLOCK_SIZE, WHITE);
					// UnloadTexture(tex);
					// UnloadImage(img);
				} break;

				default:
					break;
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

	vec2f32_t plane_middle = vec2f32_from_angle(angle);
	vec2f32_scale(&plane_middle, fov_length, &plane_middle);
	vec2f32_add(&plane_middle , &CAST_TYPE(vec2f32_t, pos), &plane_middle);

	vec2f32_t pov_plane[2] = {0};
	get_fov_plane(CAST_TYPE(vec2f32_t, pos), angle, fov_length, pov_plane);

	DrawLineV(pos, CAST_TYPE(Vector2, pov_plane[0]), WHITE);
	DrawLineV(pos, CAST_TYPE(Vector2, pov_plane[1]), WHITE);
	DrawLineV(CAST_TYPE(Vector2, pov_plane[0]), CAST_TYPE(Vector2, pov_plane[1]), WHITE);

	// DrawCircleV(CAST_TYPE(Vector2, pov_plane[0]), 1, BLUE);
	// DrawCircleV(CAST_TYPE(Vector2, plane_middle), 1, RED);
	// DrawCircleV(CAST_TYPE(Vector2, pov_plane[1]), 1, BLUE);

	DrawCircleV(pos, 1, GREEN);
}

void update_player(scene_t *scene, f32 delta_time)
{
	vec2f32_t speed = { 0, 0 };
	const vec2f32_t direction = vec2f32_from_angle(scene->player_angle);

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

	const f32 max_x = scene->width;
	const f32 new_x = scene->player_position.x + speed.x * delta_time;

	const f32 max_y = scene->height;
	const f32 new_y = (scene->player_position.y + speed.y * delta_time);

	vec2f32_t new_position = {
		.x = MAX(MIN(new_x, max_x), 0),
		.y = scene->player_position.y,
	};
	bool hit_a_block = collision_hit_a_block(scene, scene->player_position, new_position, 0, 0, 0);
	if (!hit_a_block) {
		scene->player_position.x = new_position.x;
	}

	new_position.x = scene->player_position.x;
	new_position.y = MAX(MIN(new_y, max_y), 0);
	hit_a_block = collision_hit_a_block(scene, scene->player_position, new_position, 0, 0, 0);
	if (!hit_a_block) {
		scene->player_position.y = new_position.y;
	}

	if (IsKeyDown(KEY_LEFT)) {
		scene->player_angle -= 0.75f * delta_time;
	}

	if (IsKeyDown(KEY_RIGHT)) {
		scene->player_angle += 0.75f * delta_time;
	}

	if (scene->player_angle < 0) {
		scene->player_angle = 2 * PI - scene->player_angle;
	} else if (scene->player_angle > 2 * PI) {
		scene->player_angle = scene->player_angle - 2 * PI;
	}
}

int main(void)
{
	InitWindow(SCREEN_WITDH, SCREEN_HEIGHT, "RayCast");

	Image brick_img = LoadImage("assets/textures/bricksx64.png");
	Color *cs = LoadImageColors(brick_img);

	block_t blocks[SCENE_WIDTH*SCENE_HEIGHT] = {0};
	ceil_e ceil_grid[SCENE_WIDTH*SCENE_HEIGHT] = {0};

	for (u32 i = 0; i < SCENE_WIDTH*SCENE_HEIGHT; i++) {
		ceil_grid[i] = i % 2 == 0 ? CEIL_BLUE : CEIL_RED;
	}

	scene_t scene = {
		.width = SCENE_WIDTH,
		.height = SCENE_HEIGHT,
		.blocks = blocks,
		.ceil_grid = ceil_grid,
		.player_angle = BASE_ROTATION,
	};
	scene.player_position.x = scene.width / 2.0f;// * BLOCK_SIZE / 2.f;
	scene.player_position.y = scene.height / 2.0f;// * BLOCK_SIZE / 2.f;

	u32 colors[] = {
		0xffff0000, // BLUE
		0xff0000ff, // RED
		0xff00ff00, // GREEN
		0xffffffff, // WHITE
		0xff999999, // Grey
	};
	scene.blocks[0] = (block_t) {
		.block_type = BLOCK_COLOR,
		.data = &colors[0],
	};
	scene.blocks[xy_to_index(3, 3, scene.width)] = (block_t) {
		.block_type = BLOCK_COLOR,
		.data = &colors[1],
	};
	scene.blocks[xy_to_index(2, 3, scene.width)] = (block_t) {
		.block_type = BLOCK_COLOR,
		.data = &colors[2],
	};
	scene.blocks[xy_to_index(3, 2, scene.width)] = (block_t) {
		.block_type = BLOCK_COLOR,
		.data = &colors[3],
	};
	scene.blocks[xy_to_index(1, 2, scene.width)] = (block_t) {
		.block_type = BLOCK_COLOR,
		.data = &colors[4],
	};
	// scene.blocks[2] = BLOCK_COLOR;
	// scene.blocks[4] = BLOCK_COLOR;
	// scene.blocks[6] = BLOCK_COLOR;
	// scene.blocks[8] = BLOCK_COLOR;
	// scene.blocks[10] = BLOCK_COLOR;
	// scene.blocks[12] = BLOCK_COLOR;
	// scene.blocks[14] = BLOCK_COLOR;
	// scene.blocks[16] = BLOCK_COLOR;
	// scene.blocks[18] = BLOCK_COLOR;

	// for (u32 i = 0; i < scene.width * scene.height; i ++) {
	// 	scene.blocks[i] = BLOCK_BRICKS;
	// }

	u32 textures[3][TEXTURE_SIZE*TEXTURE_SIZE] = {0};
	for (u32 x = 0; x < TEXTURE_SIZE; x++) {
		for (u32 y = 0; y < TEXTURE_SIZE; y++) {
			const u32 tex_index = TEXTURE_SIZE * y + x;
			textures[0][tex_index] = (0xff << (8 * 3)) | (0xfe * (x != y && x != TEXTURE_SIZE - y)) << (8 *  2); // RED with a black cross.
			textures[1][tex_index] = (0xff << (8 * 3)) | (0xc0 * (x % 16 && y % 16)) << (8 *  1); // RED bricks.

			textures[2][tex_index] = (cs[tex_index].a << (8 * COLOR_CHANNEL_ALPHA))
						| (cs[tex_index].r << (8 * COLOR_CHANNEL_RED))
						| (cs[tex_index].g << (8 * COLOR_CHANNEL_GREEN))
						| (cs[tex_index].b << (8 * COLOR_CHANNEL_BLUE));
		}
	}

	scene.blocks[xy_to_index(5, 2, scene.width)] = (block_t) {
		.block_type = BLOCK_BRICKS,
		.data = &textures[0],
	};
	scene.blocks[xy_to_index(7, 2, scene.width)] = (block_t) {
		.block_type = BLOCK_BRICKS,
		.data = &textures[1],
	};
	scene.blocks[xy_to_index(9, 2, scene.width)] = (block_t) {
		.block_type = BLOCK_BRICKS,
		.data = &textures[2],
	};

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
			DrawRectangleLines(0, 0, minimap_size+3, minimap_size+3, GOLD);
		EndDrawing();

		f32 delta_time = GetFrameTime();
		update_player(&scene, delta_time);
	}

	return 0;
}
