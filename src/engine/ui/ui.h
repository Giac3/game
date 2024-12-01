#pragma once

#include "../physics/physics.h"

typedef void (*On_Click)(AABB *self, void *user_data);
typedef void (*On_Toggle)(AABB *self, void *user_data);

void button(f32 x, f32 y, f32 width, f32 height, u8 border_radius, vec4 color, const char *text, On_Click on_click, void *user_data);
void block(f32 x, f32 y, f32 width, f32 height, u8 border_radius, vec4 color);
void ui_init(void);

void toggle(f32 x, f32 y, f32 width, f32 height, u8 border_radius, vec4 color, const char *text, const char *id, On_Toggle on_toggle, void *user_data);