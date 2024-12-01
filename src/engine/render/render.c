#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../global.h"
#include "render.h"
#include "../util/util.h"
#include "../ui/ui.h"
#include "render_internal.h"

static int window_width = 1280;
static int window_height = 720;
static f32 render_width = 1280 / 3;
static f32 render_height = 720 / 3 ;
static f32 scale = 3;

static u32 vao_quad;
static u32 vbo_quad;
static u32 ebo_quad;
static u32 vao_line;
static u32 vbo_line;
static u32 shader_default;
static u32 shader_rounded;
static u32 texture_color;

static u32 vao_batch;
static u32 vbo_batch;
static u32 ebo_batch;
static u32 shader_batch;


static Array_List *list_rounded_quad;
static Array_List *list_standard_quad;
static Array_List *list_quad_line;
static Array_List *list_render_pipeline;

#define MAX_BATCHES 10
static Batch batches[MAX_BATCHES];
static usize batch_count = 0;


SDL_Window *render_init(void) {
    SDL_Window *window = render_init_window(window_width, window_height);

    render_init_quad(&vao_quad, &vbo_quad, &ebo_quad);
    render_init_line(&vao_line, &vbo_line);
    render_init_batch_quads(&vao_batch, &vbo_batch, &ebo_batch);
    render_init_shaders(&shader_default, &shader_batch, &shader_rounded, render_width, render_height);
    render_init_color_texture(&texture_color);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    list_rounded_quad = array_list_create(sizeof(Rounded_Quad), 8);
    list_standard_quad = array_list_create(sizeof(Standard_Quad), 8);
    list_quad_line = array_list_create(sizeof(Quad_Line), 8);

    usize max_size = sizeof(Quad_Line);

    if (sizeof(Standard_Quad) > max_size) {
        max_size = sizeof(Standard_Quad);
    }
    if (sizeof(Rounded_Quad) > max_size) {
        max_size = sizeof(Rounded_Quad);
    }
    if (sizeof(Batch_Vertex) > max_size) {
        max_size = sizeof(Batch_Vertex);
    }

    list_render_pipeline = array_list_create(max_size + sizeof(Renderable), 8);

    stbi_set_flip_vertically_on_load(1);
    
    return window;
};

void render_begin(void) {
    glClearColor(0.08, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    list_rounded_quad->len = 0;
};

static void render_rounded_quad(Rounded_Quad *rounded_quad) {
    glUseProgram(shader_rounded);

    mat4x4 model;
    mat4x4_identity(model);

    mat4x4_translate(model, rounded_quad->position[0], rounded_quad->position[1], 0);
    mat4x4_scale_aniso(model, model, rounded_quad->size[0], rounded_quad->size[1], 1);

    glUniformMatrix4fv(
        glGetUniformLocation(shader_rounded, "model"), 1, GL_FALSE, &model[0][0]
    );
    glUniform4fv(glad_glGetUniformLocation(shader_rounded, "color"), 1, rounded_quad->color);

    glUniform2f(glGetUniformLocation(shader_rounded, "resolution"), rounded_quad->size[0], rounded_quad->size[1]);
    glUniform1f(glGetUniformLocation(shader_rounded, "border_radius"), rounded_quad->border_radius);
    glUniform2f(glGetUniformLocation(shader_rounded, "center"), rounded_quad->position[0], rounded_quad->position[1]);

    glBindVertexArray(vao_quad);

    glBindTexture(GL_TEXTURE_2D, texture_color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0); 
};

// static void render_batch(Batch_Vertex *vertices, usize count, u32 texture_id) {
//     glBindBuffer(GL_ARRAY_BUFFER, vbo_batch);
//     glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Batch_Vertex), vertices);

//     glActiveTexture(GL_TEXTURE0);
//     glBindTexture(GL_TEXTURE_2D, texture_id);

//     glUseProgram(shader_batch);
//     glBindVertexArray(vao_batch);

//     glDrawElements(GL_TRIANGLES, (count >> 2) * 6, GL_UNSIGNED_INT, NULL);
// };

static void append_quad(vec2 position, vec2 size, vec4 texture_coordinates, vec4 color, u32 texture_id, bool render_in_batch) {
    if (render_in_batch) {
        // Existing batch handling code
        // Find or create a batch for the given texture_id
        Batch *batch = NULL;
        for (usize i = 0; i < batch_count; ++i) {
            if (batches[i].texture_id == texture_id) {
                batch = &batches[i];
                break;
            }
        }
        if (batch == NULL) {
            // Create a new batch
            if (batch_count >= MAX_BATCHES) {
                ERROR_EXIT("Exceeded maximum number of batches.\n");
            }
            batch = &batches[batch_count++];
            batch->texture_id = texture_id;
            batch->vertices = array_list_create(sizeof(Batch_Vertex), 1024);  // Adjust initial capacity as needed
        }

        // Append the quad to the batch's vertex list
        vec4 uvs = {0, 0, 1, 1};
        if (texture_coordinates != NULL) {
            memcpy(uvs, texture_coordinates, sizeof(vec4));
        }

        Array_List *vertex_list = batch->vertices;

        array_list_append(vertex_list, &(Batch_Vertex) {
                .position = {position[0], position[1]},
                .uvs = {uvs[0], uvs[1]},
                .color = {color[0], color[1], color[2], color[3]},
                .border_radius = 0,
            });

        array_list_append(vertex_list, &(Batch_Vertex) {
                .position = {position[0] + size[0], position[1]},
                .uvs = {uvs[2], uvs[1]},
                .color = {color[0], color[1], color[2], color[3]},
                .border_radius = 0,
            });
        array_list_append(vertex_list, &(Batch_Vertex) {
                .position = {position[0] + size[0], position[1] + size[1]},
                .uvs = {uvs[2], uvs[3]},
                .color = {color[0], color[1], color[2], color[3]},
                .border_radius = 0,
            });
        array_list_append(vertex_list, &(Batch_Vertex) {
                .position = {position[0], position[1] + size[1]},
                .uvs = {uvs[0], uvs[3]},
                .color = {color[0], color[1], color[2], color[3]},
                .border_radius = 0,
            });


    } else {
        // Create a new batch for this quad and append it to the render pipeline
        Batch new_batch;
        new_batch.texture_id = texture_id;
        new_batch.vertices = array_list_create(sizeof(Batch_Vertex), 4);

        // Append the quad to the batch's vertex list
        vec4 uvs = {0, 0, 1, 1};
        if (texture_coordinates != NULL) {
            memcpy(uvs, texture_coordinates, sizeof(vec4));
        }

        Array_List *vertex_list = new_batch.vertices;

        array_list_append(vertex_list, &(Batch_Vertex) {
                .position = {position[0], position[1]},
                .uvs = {uvs[0], uvs[1]},
                .color = {color[0], color[1], color[2], color[3]},
                .border_radius = 0,
            });

        array_list_append(vertex_list, &(Batch_Vertex) {
                .position = {position[0] + size[0], position[1]},
                .uvs = {uvs[2], uvs[1]},
                .color = {color[0], color[1], color[2], color[3]},
                .border_radius = 0,
            });
        array_list_append(vertex_list, &(Batch_Vertex) {
                .position = {position[0] + size[0], position[1] + size[1]},
                .uvs = {uvs[2], uvs[3]},
                .color = {color[0], color[1], color[2], color[3]},
                .border_radius = 0,
            });
        array_list_append(vertex_list, &(Batch_Vertex) {
                .position = {position[0], position[1] + size[1]},
                .uvs = {uvs[0], uvs[3]},
                .color = {color[0], color[1], color[2], color[3]},
                .border_radius = 0,
            });

        Renderable renderable = {
            .type = BATCH,
            .data.batch = new_batch,
        };
        array_list_append(list_render_pipeline, &renderable);
    }
}

static void render_batch(Batch *batch) {
    glUseProgram(shader_batch);
    glBindVertexArray(vao_batch);

    Batch_Vertex *vertices = batch->vertices->items;
    usize count = batch->vertices->len;
    usize buffer_size = count * sizeof(Batch_Vertex);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_batch);

    // Orphan the buffer
    glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_DYNAMIC_DRAW);

    // Update the buffer data
    glBufferSubData(GL_ARRAY_BUFFER, 0, buffer_size, vertices);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, batch->texture_id);

    glDrawElements(GL_TRIANGLES, (count / 4) * 6, GL_UNSIGNED_INT, NULL);

    // Clear the batch for the next frame
    batch->vertices->len = 0;
}

static void batch_render_vertices() {
    glUseProgram(shader_batch);
    glBindVertexArray(vao_batch);

    for (usize i = 0; i < batch_count; ++i) {
        Batch *batch = &batches[i];
        Batch_Vertex *vertices = batch->vertices->items;
        usize count = batch->vertices->len;

        glBindBuffer(GL_ARRAY_BUFFER, vbo_batch);

        glBufferData(GL_ARRAY_BUFFER,  count * sizeof(Batch_Vertex), NULL, GL_DYNAMIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Batch_Vertex), vertices);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, batch->texture_id);

        glDrawElements(GL_TRIANGLES, (count >> 2) * 6, GL_UNSIGNED_INT, NULL);

        // Clear the batch for the next frame
        batch->vertices->len = 0;
    }
    
    // Reset batch count for the next frame
    batch_count = 0;
}

void append_quad_line(vec2 pos, vec2 size, vec4 color) {
    array_list_append(list_render_pipeline, &(Renderable){
        .type = QUAD_LINE,
        .data.quad_line = {
        .pos = {pos[0], pos[1]},
        .size = {size[0], size[1]},
        .color = {color[0], color[1], color[2], color[3]}
    },
    });
};

void render_quad_line(Quad_Line *quad_line) {
    vec2 points[4] = {
        {quad_line->pos[0] - quad_line->size[0] * 0.5, quad_line->pos[1] - quad_line->size[1] * 0.5},
        {quad_line->pos[0] + quad_line->size[0] * 0.5, quad_line->pos[1] - quad_line->size[1] * 0.5},
        {quad_line->pos[0] + quad_line->size[0] * 0.5, quad_line->pos[1] + quad_line->size[1] * 0.5},
        {quad_line->pos[0] - quad_line->size[0] * 0.5, quad_line->pos[1] + quad_line->size[1] * 0.5},
    };

    render_line_segment(points[0], points[1], quad_line->color);
    render_line_segment(points[1], points[2], quad_line->color);
    render_line_segment(points[2], points[3], quad_line->color);
    render_line_segment(points[3], points[0], quad_line->color);
};


static void render_cursor() {
    f32 mouseX_world = global.input.mouseX;
    f32 mouseY_world = global.input.mouseY;
    vec2 mousePos_world = {mouseX_world, mouseY_world};
    Quad_Line quad_line = {
        .pos = {mouseX_world, mouseY_world},
        .size = {2,2},
        .color = {BLUE[0], BLUE[1], BLUE[2], BLUE[3]},
    };

    render_quad_line(&quad_line);
}


void append_standard_quad(f32 *aabb, vec4 color) {
     
    array_list_append(list_render_pipeline, &(Renderable){
        .type = STANDARD_QUAD,
        .data.standard_quad = {
        .aabb = aabb,
        .color = {color[0], color[1], color[2], color[3]}
    },
    });
};

void render_standard_quad_line(vec2 pos, vec2 size, vec4 color) {
    vec2 points[4] = {
        {pos[0] - size[0] * 0.5, pos[1] - size[1] * 0.5},
        {pos[0] + size[0] * 0.5, pos[1] - size[1] * 0.5},
        {pos[0] + size[0] * 0.5, pos[1] + size[1] * 0.5},
        {pos[0] - size[0] * 0.5, pos[1] + size[1] * 0.5},
    };

    render_line_segment(points[0], points[1], color);
    render_line_segment(points[1], points[2], color);
    render_line_segment(points[2], points[3], color);
    render_line_segment(points[3], points[0], color);
};


static void render_standard_quad(Standard_Quad *standard_quad) {
    vec2 size;
    vec2_scale(size, &standard_quad->aabb[2], 2);
    render_standard_quad_line(&standard_quad->aabb[0], size, standard_quad->color);
};

void render_end(SDL_Window *window) {
    batch_render_vertices();

    for (usize i = 0; i < list_render_pipeline->len; ++i) {

        Renderable *renderable = array_list_get(list_render_pipeline, i);

        switch (renderable->type){
        case ROUNDED_QUAD:
            render_rounded_quad(&renderable->data.rounded_quad);
            break;
        case STANDARD_QUAD:
            render_standard_quad(&renderable->data.standard_quad);
            break;
        case QUAD_LINE:
            render_quad_line(&renderable->data.quad_line);
            break;
        case BATCH:
            render_batch(&renderable->data.batch);
            break;
        default:
            break;
        }
    };

    list_render_pipeline->len = 0;

    render_cursor();
    SDL_GL_SwapWindow(window);
}

void render_quad(vec2 pos, vec2 size, vec4 color) {
    glUseProgram(shader_default);

    mat4x4 model;
    mat4x4_identity(model);

    mat4x4_translate(model, pos[0], pos[1], 0);
    mat4x4_scale_aniso(model, model, size[0], size[1], 1);

    glUniformMatrix4fv(
        glGetUniformLocation(shader_default, "model"), 1, GL_FALSE, &model[0][0]
    );
    glUniform4fv(glad_glGetUniformLocation(shader_default, "color"), 1, color);

    glBindVertexArray(vao_quad);

    glBindTexture(GL_TEXTURE_2D, texture_color);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0); 
};

void append_rounded_quad(vec2 pos, vec2 size, vec4 color, u8 border_radius) {
    // array_list_append(list_rounded_quad, &rounded_quad);
    array_list_append(list_render_pipeline, &(Renderable){
        .type = ROUNDED_QUAD,
        .data.rounded_quad = {
        .position = {pos[0], pos[1]},
        .size = {size[0], size[1]},
        .color = {color[0], color[1], color[2], color[3]},
        .border_radius = border_radius
    },
    });
}

void render_line_segment(vec2 start, vec2 end, vec4 color) {
    glUseProgram(shader_default);
    glLineWidth(3);

    f32 x = end[0] - start[0];
    f32 y = end[1] - start[1];
    f32 line[6] = {0, 0, 0, x, y, 0};

    mat4x4 model;
    mat4x4_translate(model, start[0], start[1], 0);

    glUniformMatrix4fv(glGetUniformLocation(shader_default, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform4fv(glGetUniformLocation(shader_default, "color"), 1, color);

    glBindTexture(GL_TEXTURE_2D, texture_color);
    glBindVertexArray( vao_line);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_line);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line), line);
    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(0);
};


f32 render_get_scale() {
    return scale;
};

void render_sprite_sheet_init(Sprite_Sheet *sprite_sheet, const char *path, f32 cell_width, f32 cell_height) {
     glGenTextures(1, &sprite_sheet->texture_id);
     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_2D, sprite_sheet->texture_id);

     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

     int width, height, channel_count;
     u8 *image_data = stbi_load(path, &width, &height, &channel_count, 0);
     if(!image_data) {
        ERROR_EXIT("Failed to load image: %s\n", path);
     };
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data); 
     stbi_image_free(image_data);

     sprite_sheet->width = (f32)width;
     sprite_sheet->height = (f32)height;
     sprite_sheet->cell_width = (f32)cell_width;
     sprite_sheet->cell_height = (f32)cell_height;
};

static void calculate_sprite_texture_coordinate(vec4 result, f32 row, f32 column, f32 texture_width, f32 texture_height, f32 cell_width, f32 cell_height) {
    f32 w = 1.0 / (texture_width / cell_width);
    f32 h = 1.0 / (texture_height / cell_height);
    f32 x = column * w;
    f32 y = row * h;
    result[0] = x;
    result[1] = y;
    result[2] = x + w;
    result[3] = y + h;
};

void render_sprite_sheet_frame(Sprite_Sheet *sprite_sheet, f32 row, f32 column, vec2 position, bool is_flipped, bool render_in_batch) {
    vec4 uvs;
    calculate_sprite_texture_coordinate(uvs, row, column, sprite_sheet->width, sprite_sheet->height, sprite_sheet->cell_width, sprite_sheet->cell_height);

    if (is_flipped) {
        f32 tmp = uvs[0];
        uvs[0] = uvs[2];
        uvs[2] = tmp;
    };

    vec2 size = {sprite_sheet->cell_width, sprite_sheet->cell_height};
    vec2 bottom_left = {
        position[0] - size[0] * 0.5,
        position[1] - size[1] * 0.5,
    };
    
    append_quad(bottom_left, size, uvs, WHITE, sprite_sheet->texture_id, render_in_batch);
    
};

void render_textured_quad_with_texture_id(vec2 position, vec2 size, vec4 uv_rect, vec4 color, u32 texture_id) {
    append_quad(position, size, uv_rect, color, texture_id, true);
}

void calculate_tile_uv(vec4 result, Sprite_Sheet *sprite_sheet, int row, int column) {
    float u_min = column * sprite_sheet->cell_width / sprite_sheet->width;
    float v_min = row * sprite_sheet->cell_height / sprite_sheet->height;
    float u_max = (column + 1) * sprite_sheet->cell_width / sprite_sheet->width;
    float v_max = (row + 1) * sprite_sheet->cell_height / sprite_sheet->height;

    result[0] = u_min;
    result[1] = v_min;
    result[2] = u_max;
    result[3] = v_max;
}

u32 create_tile_texture(Sprite_Sheet *sprite_sheet, const char *path, int row, int column) {
    // Calculate the pixel coordinates of the tile in the tileset image
    int x = column * sprite_sheet->cell_width;
    int y = row * sprite_sheet->cell_height;
    int width = sprite_sheet->cell_width;
    int height = sprite_sheet->cell_height;

    // Load the tileset image again
    int img_width, img_height, img_channels;
    u8 *image_data = stbi_load(path, &img_width, &img_height, &img_channels, 0);
    if (!image_data) {
        ERROR_EXIT("Failed to load image: %s\n", path);
    }

    // Create a buffer for the tile image data
    int tile_channels = 4;  // We will use RGBA format
    u8 *tile_data = malloc(width * height * tile_channels);

    // Copy the pixel data for the selected tile
    for (int i = 0; i < height; ++i) {
        int src_row = y + i;
        int dst_row = i;

        memcpy(
            tile_data + dst_row * width * tile_channels,
            image_data + src_row * img_width * tile_channels + x * tile_channels,
            width * tile_channels
        );
    }

    stbi_image_free(image_data);

    // Create a new texture for the tile
    u32 texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // Repeat texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  // Repeat texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Important for pixel art
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Upload the tile's pixel data to the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tile_data);

    free(tile_data);

    return texture_id;
}