#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include "raylib.h"

#include "src/image.h"
#include "src/types.h"
#include "src/utils.h"
#include "src/vec2f32.h"

#include "src/defines.h"

#include "src/render.h"
#include "src/scene.h"
#include "src/collision.h"

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
	// TODO: Refactor to a separated function on scene module
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

	speed.x *= delta_time;
	speed.y *= delta_time;

	const f32 max_x = scene->width;
	const f32 new_x = scene->player_position.x + speed.x;

	const f32 max_y = scene->height;
	const f32 new_y = scene->player_position.y + speed.y;

	vec2f32_t new_position = {
		.x = MAX(MIN(new_x, max_x), 0),
		.y = scene->player_position.y,
	};
	collision_block_t collision_block = collision_block_empty();
	bool hit_a_block = collision_hit_a_block(scene, scene->player_position, new_position, &collision_block);

	bool is_a_portal_block = collision_block.block_ptr == scene->portal1.block_src
		|| collision_block.block_ptr == scene->portal2.block_src;
	if (!hit_a_block) {
		scene->player_position.x = new_position.x;
	}
	else if (is_a_portal_block) {
		scene_teleport_player(scene, &collision_block);
		// TODO: Fix this
		return;
	} else {
		switch (collision_block.face) {
			case BLOCK_FACE_RIGHT:
				collision_block.hit.x += 0.1;
				break;
			case BLOCK_FACE_LEFT:
				collision_block.hit.x -= 0.1;
				break;
			default:
				break;
		}
		scene->player_position.x = collision_block.hit.x;
	}

	new_position.x = scene->player_position.x;
	new_position.y = MAX(MIN(new_y, max_y), 0);
	hit_a_block = collision_hit_a_block(scene, scene->player_position, new_position, &collision_block);

	is_a_portal_block = collision_block.block_ptr == scene->portal1.block_src
		|| collision_block.block_ptr == scene->portal2.block_src;
	if (!hit_a_block) {
		scene->player_position.y = new_position.y;
	}
	else if (is_a_portal_block) {
		scene_teleport_player(scene, &collision_block);
		// TODO: Fix this
		return;
	} else {
		switch (collision_block.face) {
			case BLOCK_FACE_UP:
				collision_block.hit.y -= 0.1;
				break;
			case BLOCK_FACE_DOWN:
				collision_block.hit.y += 0.1;
				break;
			default:
				break;
		}
		scene->player_position.y = collision_block.hit.y;
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

	if (IsKeyPressed(KEY_UP)) {
		scene_place_teleport(scene, PORTAL_1);
	}
	if (IsKeyPressed(KEY_DOWN)) {
		scene_place_teleport(scene, PORTAL_2);
	}
}

int main(void)
{
	InitWindow(SCREEN_WITDH, SCREEN_HEIGHT, "RayCast");

	Image brick_img = LoadImage("assets/textures/bricksx64.png");
	Color *cs = LoadImageColors(brick_img);

	Image portal1_img = LoadImage("assets/textures/portal1.png");
	Color *portal1_cs = LoadImageColors(portal1_img);
	Image portal2_img = LoadImage("assets/textures/portal2.png");
	Color *portal2_cs = LoadImageColors(portal2_img);
	u32 portal1_pixels[TEXTURE_SIZE*TEXTURE_SIZE] = {0};
	u32 portal2_pixels[TEXTURE_SIZE*TEXTURE_SIZE] = {0};
	for (u32 x = 0; x < TEXTURE_SIZE; x++) {
		for (u32 y = 0; y < TEXTURE_SIZE; y++) {
			const u32 tex_index = TEXTURE_SIZE * y + x;
			portal1_pixels[tex_index] = (portal1_cs[tex_index].a << (8 * COLOR_CHANNEL_ALPHA))
						 | (portal1_cs[tex_index].r << (8 * COLOR_CHANNEL_RED))
						 | (portal1_cs[tex_index].g << (8 * COLOR_CHANNEL_GREEN))
						 | (portal1_cs[tex_index].b << (8 * COLOR_CHANNEL_BLUE));
			portal2_pixels[tex_index] = (portal2_cs[tex_index].a << (8 * COLOR_CHANNEL_ALPHA))
						 | (portal2_cs[tex_index].r << (8 * COLOR_CHANNEL_RED))
						 | (portal2_cs[tex_index].g << (8 * COLOR_CHANNEL_GREEN))
						 | (portal2_cs[tex_index].b << (8 * COLOR_CHANNEL_BLUE));
		}
	}
	UnloadImage(portal1_img);
	UnloadImage(portal2_img);

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
		.portal1 = {0},
		.portal2 = {0},
	};
	scene.player_position.x = scene.width / 2.0f;
	scene.player_position.y = scene.height / 2.0f;

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
	scene.blocks[xy_to_index(scene.width-1, 0, scene.width)] = (block_t) {
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
	scene.portal1 = (portal_t){
		.position = (vec2u32_t) { .x = 5, .y = 2 },
		.block_src = &scene.blocks[xy_to_index(5, 2, scene.width)],
		.block_dest = &scene.blocks[xy_to_index(9, 2, scene.width)],
		.face = BLOCK_FACE_DOWN,
		.pixels = portal1_pixels,
	};
	scene.portal2 = (portal_t){
		.position = (vec2u32_t) { .x = 9, .y = 2 },
		.block_src = &scene.blocks[xy_to_index(9, 2, scene.width)],
		.block_dest = &scene.blocks[xy_to_index(5, 2, scene.width)],
		.face = BLOCK_FACE_DOWN,
		.pixels = portal2_pixels,
	};

	RenderTexture2D minimap = LoadRenderTexture(scene.width * BLOCK_SIZE, scene.height * BLOCK_SIZE);

	vec2f32_t speed = { 1.0f, 1.0f };
	const u32 minimap_size = 320;

	u32 pixel_buffer[SCREEN_WITDH*SCREEN_HEIGHT] = {0};
	image_t game_image = image_create(SCREEN_WITDH, SCREEN_HEIGHT, pixel_buffer);
	image_clear(&game_image, 0xff000000);
	RenderTexture2D tex_game_image = LoadRenderTexture(SCREEN_WITDH, SCREEN_HEIGHT);

	SetTargetFPS(75);
	while(!WindowShouldClose()) {
		image_clear(&game_image, 0xff000000);
		render_scene_on_image(&scene, SCREEN_WITDH, SCREEN_HEIGHT, &game_image);
		image_draw_cross(&game_image, 8, 2, 0xffffffff);
		UpdateTexture(tex_game_image.texture, game_image.pixel_buffer);

		BeginTextureMode(minimap);
			ClearBackground(BLACK);
			draw_grid(scene.blocks, scene.width, scene.height);
			//draw_step_function(&scene);
			draw_player_view(*(Vector2*)&scene.player_position, scene.player_angle);
		EndTextureMode();

		BeginDrawing();
			ClearBackground(BLACK);

			// TODO: Test game scale on window
			DrawTexturePro(tex_game_image.texture,
					(Rectangle) {.x = 0, .y = 0, .width = game_image.width, .height = game_image.height },
					(Rectangle) {.x = 0, .y = 0, .width = GetScreenWidth(), .height = GetScreenHeight()},
					(Vector2) {0, 0},
					0, WHITE);

			DrawTexturePro(minimap.texture,
					(Rectangle) {.x = 0, .y = 0, .width = minimap.texture.width, .height = -minimap.texture.height},
					(Rectangle) {.x = 2, .y = 2, .width = minimap_size, .height = minimap_size},
					// (Rectangle) {.x = 2, .y = 2, .width = GetScreenWidth(), .height = GetScreenHeight()},
					(Vector2) {0, 0},
					0,
					WHITE);
			DrawRectangleLines(0, 0, minimap_size+3, minimap_size+3, GOLD);
			DrawFPS(GetScreenWidth() - 128, 16);
		EndDrawing();

		f32 delta_time = GetFrameTime();
		update_player(&scene, delta_time);
	}

	return 0;
}
