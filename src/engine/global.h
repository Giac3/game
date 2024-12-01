#pragma once

#include "util/hashmap.h"
#include "render/render.h"
#include "config/config.h"
#include "input/input.h"
#include "time/time.h"


typedef struct window {
    f32 width;
    f32 height;
} Window_State;

typedef struct global {
    Config_State config;
    Input_State input;
    Time_State time;
    Window_State window;
    hashmap_t ui;
    Sprite_Sheet sprite_sheet_tileset;
} Global;

extern Global global;

