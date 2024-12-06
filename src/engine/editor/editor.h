#pragma once

#include "../types.h"
#include "../render/render.h"


typedef enum editor_action {
    CREATING,
    RESIZING,
    MOVING,
    IDLE,
} Editor_Action;


typedef struct editor_state {
    f32 startingX;
    f32 startingY;
    Editor_Action action;
    vec2 selected_sprite_coords;
    usize active_body;
    int sprite_sheet_grid_y_offset;
    vec2 offset;
    vec2 initial_size;
    vec2 initial_position;
    Array_List *list_tiled_static_bodies;
} Editor_State;

typedef struct tile_coordinates {
    int row;
    int column;
} Tile_Coordinates;

typedef struct tiled_static_body {
    Tile_Coordinates tile_coordinates;
    usize static_body;
} Tiled_Static_Body;

void level_editor_render(void);
void load_level(void);
void editor_init(void);