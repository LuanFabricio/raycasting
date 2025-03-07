#ifndef __SRC_RENDER_H__
#define __SRC_RENDER_H__

#include "types.h"

void get_fov_plane(const vec2f32_t pos, const f32 angle, const f32 scale, vec2f32_t out[2]);
u32 render_get_texture_x(const vec2f32_t *hit_point, const block_face_e block_face, const u32 width);
u32 render_get_texture_y(const u32 screen_y, const u32 render_y, const f32 text_height_prop);
u32 render_get_texture_color_index(const u32 tex_x, const u32 tex_y, const u32 tex_height);
void render_scene_on_image(scene_t *scene, const u32 screen_width, const u32 screen_height, image_t* image);

#endif // __SRC_RENDER_H__
