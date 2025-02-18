#include <dlfcn.h>

#include <raylib.h>
#include <stdlib.h>

#include "src/defines.h"
#include "src/types.h"

typedef scene_t* init_scene_fn();
typedef void loop_fn(scene_t *scene, image_t *game_image, RenderTexture2D *tex_game_image, RenderTexture2D *minimap, const u32 minimap_size);

int main(void)
{
	InitWindow(SCREEN_WITDH, SCREEN_HEIGHT, "RayCast");

	void* module = dlopen("build/core.so", RTLD_NOW);
	init_scene_fn *init_scene = dlsym(module, "init_scene");
	loop_fn *loop = dlsym(module, "loop");
	scene_t *scene = init_scene();

	RenderTexture2D minimap = LoadRenderTexture(scene->width * BLOCK_SIZE, scene->height * BLOCK_SIZE);

	const u32 minimap_size = 320;

	u32 pixel_buffer[SCREEN_WITDH*SCREEN_HEIGHT] = {0};
	image_t game_image = {
		.width = SCREEN_WITDH,
		.height = SCREEN_HEIGHT,
		.pixel_buffer = pixel_buffer
	};
	RenderTexture2D tex_game_image = LoadRenderTexture(SCREEN_WITDH, SCREEN_HEIGHT);

	SetTargetFPS(70);
	const char *reloading_text = "Reloading...";
	const Vector2 text_size = MeasureTextEx(GetFontDefault(), reloading_text, 32.f, 3.2f);

	while(!WindowShouldClose()) {
		loop(scene, &game_image, &tex_game_image, &minimap, minimap_size);

		if (IsKeyPressed(KEY_R)) {
			BeginDrawing();
				DrawText("Reloading...", 10, GetScreenHeight()-text_size.y-5, 32, WHITE);
			EndDrawing();

			dlclose(module);
			system("make");

			module = dlopen("build/core.so", RTLD_NOW);
			loop = dlsym(module, "loop");
		}
	}

	return 0;
}
