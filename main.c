#include <stdio.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include "raylib.h"

#include "src/image.h"
#include "src/types.h"
#include "src/utils.h"
#include "src/vec2f32.h"

#include "src/defines.h"

#include "src/render.h"
#include "src/scene.h"

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
	const vec2f32_t direction = vec2f32_from_angle(scene->player.angle);
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
	scene_move_player(scene, speed);

	f32 angle = 0;
	if (IsKeyDown(KEY_LEFT)) {
		angle -= 0.75f * delta_time;
	}

	if (IsKeyDown(KEY_RIGHT)) {
		angle += 0.75f * delta_time;
	}
	scene_rotate_player_camera(scene, angle);

	if (IsKeyPressed(KEY_UP)) {
		scene_place_teleport(scene, PORTAL_1);
	}
	if (IsKeyPressed(KEY_DOWN)) {
		scene_place_teleport(scene, PORTAL_2);
	}
}

void load_textures(texture_map_t *tex_map)
{
	Image brick_img = LoadImage("assets/textures/bricksx64.png");
	Color *bricks_cs = LoadImageColors(brick_img);

	Image portal1_img = LoadImage("assets/textures/portal1.png");
	Color *portal1_cs = LoadImageColors(portal1_img);
	Image portal2_img = LoadImage("assets/textures/portal2.png");
	Color *portal2_cs = LoadImageColors(portal2_img);
	Image debug_img = LoadImage("assets/textures/debug.png");
	Color *debug_cs = LoadImageColors(debug_img);

	const u32 tex_bytes = sizeof(u32) * TEXTURE_SIZE * TEXTURE_SIZE;
	tex_map->cross_blue = malloc(tex_bytes);
	tex_map->bricks_red = malloc(tex_bytes);
	tex_map->brick_img = malloc(tex_bytes);
	tex_map->portal1 = malloc(tex_bytes);
	tex_map->portal2 = malloc(tex_bytes);
	tex_map->debug = malloc(tex_bytes);
	for (u32 y = 0; y < TEXTURE_SIZE; y++) {
		for (u32 x = 0; x < TEXTURE_SIZE; x++) {
			const u32 tex_index = TEXTURE_SIZE * y + x;

			tex_map->cross_blue[tex_index] = (0xff << (8 * 3)) | (0xfe * (x != y && x != TEXTURE_SIZE - y)) << (8 *  2); // RED with a black cross.
			tex_map->bricks_red[tex_index] = (0xff << (8 * 3)) | (0xc0 * (x % 16 && y % 16)) << (8 *  1); // RED bricks.

			tex_map->brick_img[tex_index] = (bricks_cs[tex_index].a << (8 * COLOR_CHANNEL_ALPHA))
						| (bricks_cs[tex_index].r << (8 * COLOR_CHANNEL_RED))
						| (bricks_cs[tex_index].g << (8 * COLOR_CHANNEL_GREEN))
						| (bricks_cs[tex_index].b << (8 * COLOR_CHANNEL_BLUE));

			tex_map->portal1[tex_index] = (portal1_cs[tex_index].a << (8 * COLOR_CHANNEL_ALPHA))
						 | (portal1_cs[tex_index].r << (8 * COLOR_CHANNEL_RED))
						 | (portal1_cs[tex_index].g << (8 * COLOR_CHANNEL_GREEN))
						 | (portal1_cs[tex_index].b << (8 * COLOR_CHANNEL_BLUE));
			tex_map->portal2[tex_index] = (portal2_cs[tex_index].a << (8 * COLOR_CHANNEL_ALPHA))
						 | (portal2_cs[tex_index].r << (8 * COLOR_CHANNEL_RED))
						 | (portal2_cs[tex_index].g << (8 * COLOR_CHANNEL_GREEN))
						 | (portal2_cs[tex_index].b << (8 * COLOR_CHANNEL_BLUE));
			tex_map->debug[tex_index] = (debug_cs[tex_index].a << (8 * COLOR_CHANNEL_ALPHA))
						 | (debug_cs[tex_index].r << (8 * COLOR_CHANNEL_RED))
						 | (debug_cs[tex_index].g << (8 * COLOR_CHANNEL_GREEN))
						 | (debug_cs[tex_index].b << (8 * COLOR_CHANNEL_BLUE));
		}
	}

	UnloadImage(portal1_img);
	UnloadImage(portal2_img);
	UnloadImage(debug_img);

}

int main(void)
{
	InitWindow(SCREEN_WITDH, SCREEN_HEIGHT, "RayCast");

	texture_map_t tex_map = {0};
	load_textures(&tex_map);
	block_t blocks[SCENE_WIDTH*SCENE_HEIGHT] = {0};

	scene_t scene = {
		.width = SCENE_WIDTH,
		.height = SCENE_HEIGHT,
		.blocks = blocks,
		.player = {
			.position = {0},
			.angle = BASE_ROTATION,
			.type = ENTITY_PLAYER,
		},
		.portal1 = {0},
		.portal2 = {0},
		.entities = {
			.data = {0},
			.lenght = 0,
		},
		.tex_map = tex_map,
	};
	scene.player.position.x = scene.width / 2.0f;
	scene.player.position.y = scene.height / 2.0f;

	scene.entities.data[0].type = ENTITY_ENEMY;
	scene.entities.data[0].position.x = 5.5;
	scene.entities.data[0].position.y = 5.0;
	scene.entities.data[0].angle = 0.f;
	scene.entities.lenght = 1;

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

	scene.blocks[xy_to_index(5, 2, scene.width)] = (block_t) {
		.block_type = BLOCK_BRICKS,
		.data = scene.tex_map.cross_blue,
	};
	scene.blocks[xy_to_index(7, 2, scene.width)] = (block_t) {
		.block_type = BLOCK_BRICKS,
		.data = scene.tex_map.bricks_red,
	};
	scene.blocks[xy_to_index(9, 2, scene.width)] = (block_t) {
		.block_type = BLOCK_BRICKS,
		.data = scene.tex_map.brick_img,
	};
	scene.portal1 = (portal_t){
		.position = (vec2u32_t) { .x = 5, .y = 2 },
		.block_src = &scene.blocks[xy_to_index(5, 2, scene.width)],
		.block_dest = &scene.blocks[xy_to_index(9, 2, scene.width)],
		.face = BLOCK_FACE_DOWN,
		.pixels = scene.tex_map.portal1,
	};
	scene.portal2 = (portal_t){
		.position = (vec2u32_t) { .x = 9, .y = 2 },
		.block_src = &scene.blocks[xy_to_index(9, 2, scene.width)],
		.block_dest = &scene.blocks[xy_to_index(5, 2, scene.width)],
		.face = BLOCK_FACE_DOWN,
		.pixels = scene.tex_map.portal2,
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
			draw_player_view(*(Vector2*)&scene.player.position, scene.player.angle);
		EndTextureMode();

		BeginDrawing();
			ClearBackground(BLACK);

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
