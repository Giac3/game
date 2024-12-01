#pragma once



#include <SDL2/SDL.h>
#include <linmath.h>
#include <stdbool.h>

#include "../types.h"
#include "../util/util.h"


typedef struct batch_vertex {
    vec2 position;
    vec2 uvs;
    vec4 color;
    u8 border_radius; 
} Batch_Vertex;

typedef struct rounded_quad {
    vec2 position;
    vec2 size;
    vec4 color;
    u8 border_radius; 
} Rounded_Quad;

typedef struct standard_quad {
    f32 *aabb;
    vec4 color;
} Standard_Quad;

typedef struct quad_line {
    vec2 pos;
    vec2 size;
    vec4 color;
} Quad_Line;

typedef enum renderable_type {
    QUAD_LINE,
    STANDARD_QUAD,
    ROUNDED_QUAD,
    BATCH,
} Renderable_Type;

typedef struct {
    Array_List *vertices;
    u32 texture_id;
} Batch;

typedef struct renderable {
    Renderable_Type type;
    union {
        Rounded_Quad rounded_quad;
        Standard_Quad standard_quad;
        Quad_Line quad_line;
        Batch batch;
    } data;
} Renderable;


typedef struct sprite_sheet {
    f32 width;
    f32 height;
    f32 cell_width;
    f32 cell_height;
    u32 texture_id;
} Sprite_Sheet;

#define MAX_BATCH_QUADS 10000
#define MAX_BATCH_VERTICES 40000
#define MAX_BATCH_ELEMENTS 60000

SDL_Window *render_init(void);
void render_begin(void);
void render_end(SDL_Window *window);
void render_quad(vec2 pos, vec2 size, vec4 color);
void append_rounded_quad(vec2 pos, vec2 size, vec4 color, u8 border_radius);
void append_quad_line(vec2 pos, vec2 size, vec4 color);
void render_line_segment(vec2 start, vec2 end, vec4 color);
void append_standard_quad(f32 *aabb, vec4 color);
f32 render_get_scale();

void render_sprite_sheet_init(Sprite_Sheet *sprite_sheet, const char *path, f32 cell_width, f32 cell_height);
void render_sprite_sheet_frame(Sprite_Sheet *sprite_sheet, f32 row, f32 column, vec2 position, bool is_flipped, bool render_in_batch);
u32 create_tile_texture(Sprite_Sheet *sprite_sheet, const char *path, int row, int column);
void render_textured_quad_with_texture_id(vec2 position, vec2 size, vec4 uv_rect, vec4 color, u32 texture_id);
void calculate_tile_uv(vec4 result, Sprite_Sheet *sprite_sheet, int row, int column);