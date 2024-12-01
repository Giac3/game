#pragma once

#include "../types.h"
#include "../render/render.h"

typedef struct tile_coordinates {
    int row;
    int column;
} Tile_Coordinates;

typedef struct tiled_static_body {
    Tile_Coordinates tile_coordinates;
    usize static_body;
} Tiled_Static_Body;

void level_editor_render(void);
void load_level();
void editor_init(void);