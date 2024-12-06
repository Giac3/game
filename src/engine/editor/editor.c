#include "editor.h"
#include "../types.h"
#include "../render/render.h"
#include "../util/util.h"
#include "../ui/ui.h"
#include "../physics/physics.h"
#include "../global.h"
#include <stdbool.h>
#include "../io/io.h"
#include <stdbool.h>


static Editor_State editor_state;

void editor_init(void) {
    editor_state.list_tiled_static_bodies = array_list_create(sizeof(Tiled_Static_Body), 8);
    editor_state.selected_sprite_coords[0] = 0;
    editor_state.selected_sprite_coords[1] = 0;
    editor_state.sprite_sheet_grid_y_offset = 0;
    editor_state.active_body = (usize)-1;
    editor_state.action = IDLE;

}

void select_sprite(AABB *aabb, void *user_data) {
    if (user_data != NULL) {
        editor_state.selected_sprite_coords[0] = ((Tile_Coordinates*)user_data)->column;
        editor_state.selected_sprite_coords[1] = ((Tile_Coordinates*)user_data)->row;
        free(user_data);
    }
}

void render_sprite_sheet_grid(Sprite_Sheet *sprite_sheet, vec2 start_position, float spacing) {
    int total_columns = (int)(sprite_sheet->width / sprite_sheet->cell_width);
    int total_rows = (int)(sprite_sheet->height / sprite_sheet->cell_height);
    int total_frames = total_columns * total_rows;

    for (int frame_index = 0; frame_index < total_frames; ++frame_index) {
        // Calculate row and column based on the frame index
        int row = frame_index / total_columns;
        int column = frame_index % total_columns;

        // Position sprites in a single column
        float x = start_position[0];
        float y = start_position[1] - frame_index * (sprite_sheet->cell_height + spacing) + editor_state.sprite_sheet_grid_y_offset;

        vec2 position = { x, y };

        // Render each sprite frame at the calculated position
        render_sprite_sheet_frame(sprite_sheet, row, column, position, false, false);

        // Prepare tile coordinates for the button callback
        Tile_Coordinates *tile_coords = malloc(sizeof(Tile_Coordinates));
        tile_coords->row = row;
        tile_coords->column = column;

        // Place the button next to the sprite
        float button_x = x + sprite_sheet->cell_width + spacing;
        float button_y = y; // Same y-coordinate as the sprite

        if (editor_state.selected_sprite_coords[1] == row && editor_state.selected_sprite_coords[0] == column) {
            button(
            button_x,
            button_y,
            sprite_sheet->cell_width / 2,
            sprite_sheet->cell_height / 2,
            2,
            GREEN,
            "Select",
            select_sprite,
            tile_coords
        );
        } else {
            button(
            button_x,
            button_y,
            sprite_sheet->cell_width / 2,
            sprite_sheet->cell_height / 2,
            2,
            WHITE,
            "Select",
            select_sprite,
            tile_coords
        );
        };


        // Optionally, render a quad over the sprite (if needed)
        vec2 tile_size = { sprite_sheet->cell_width, sprite_sheet->cell_height };
        render_quad(position, tile_size, WHITE);
    }
}



void save_level_item(Array_List *list, const char* path) {
    // Calculate total size: size of Array_List structure + size of items
    size_t total_size = sizeof(Array_List) + list->capacity * list->item_size;

    // Allocate buffer to hold the serialized data
    void *buffer = malloc(total_size);

    // Copy the Array_List structure (without the items pointer)
    memcpy(buffer, list, sizeof(Array_List));

    // Replace the items pointer with a relative offset or NULL
    ((Array_List *)buffer)->items = NULL;

    // Copy the items after the Array_List structure
    memcpy((char *)buffer + sizeof(Array_List), list->items, list->capacity * list->item_size);

    // Write the buffer to the file
    int saved = io_file_write(buffer, total_size, path);

    // Free the buffer
    free(buffer);

    if (saved == 1) {
        ERROR_EXIT("Level save failed");
    }
}

void load_level() {
    physics_static_body_load_from_bin("./static-bodies.bin");

    File file = io_file_read("./tiled-static-bodies.bin");

    if (!file.is_valid) {
        ERROR_EXIT("Invalid file");
    }

    // Reconstruct the Array_List from the serialized data
    Array_List *list = malloc(sizeof(Array_List));
    memcpy(list, file.data, sizeof(Array_List));

    // Allocate memory for the items
    list->items = malloc(list->capacity * list->item_size);

    // Copy the items from the file data
    memcpy(list->items, (char *)file.data + sizeof(Array_List), list->capacity * list->item_size);

    editor_state.list_tiled_static_bodies = list;

    // Free the file data if necessary
    // free(file.data);
}

void render_tiled_static_body(Static_Body *static_body, Sprite_Sheet *sprite_sheet, int row, int col) {
    // Calculate the size of the static body
    vec2 size;
    vec2_scale(size, static_body->aabb.half_size, 2.0f);

    // Calculate the minimum (left, bottom) positions of the AABB
    float min_x = static_body->aabb.position[0] - static_body->aabb.half_size[0];
    float min_y = static_body->aabb.position[1] - static_body->aabb.half_size[1];

    // Compute the number of tiles needed along the X and Y axes
    int tiles_x = (int)ceil(size[0] / sprite_sheet->cell_width);
    int tiles_y = (int)ceil(size[1] / sprite_sheet->cell_height);

    // Loop over the Y axis (rows)
    for (int y = 0; y < tiles_y; y++) {
        // Loop over the X axis (columns)
        for (int x = 0; x < tiles_x; x++) {
            // Calculate the position of the current tile
            float tile_x = min_x + x * sprite_sheet->cell_width + sprite_sheet->cell_width * 0.5f;
            float tile_y = min_y + y * sprite_sheet->cell_height + sprite_sheet->cell_height * 0.5f;

            vec2 position = { tile_x, tile_y };

            render_sprite_sheet_frame(sprite_sheet, row, col, position, false, true);
            vec2 tile_size = { sprite_sheet->cell_width, sprite_sheet->cell_height };
            render_quad(position, tile_size, WHITE);
        }
    }
}


void compute_resized_aabb(vec2 initial_position, vec2 initial_size, vec2 starting_mouse_pos, vec2 current_mouse_pos, vec2 *out_position, vec2 *out_half_size) {
    f32 dx = current_mouse_pos[0] - starting_mouse_pos[0];
    f32 dy = current_mouse_pos[1] - starting_mouse_pos[1];

    f32 new_width = initial_size[0] + dx;
    f32 new_height = initial_size[1] + dy;

    vec2 size = {fabsf(new_width), fabsf(new_height)};

    vec2 position = {initial_position[0], initial_position[1]};

    // Adjust position based on resizing direction
    if (new_width < 0) {
        position[0] += dx * 0.5f;
    } else {
        position[0] += dx * 0.5f;
    }

    if (new_height < 0) {
        position[1] += dy * 0.5f;
    } else {
        position[1] += dy * 0.5f;
    }

    (*out_position)[0] = position[0];
    (*out_position)[1] = position[1];
    (*out_half_size)[0] = size[0] * 0.5f;
    (*out_half_size)[1] = size[1] * 0.5f;
}

void scroll_up_sprite_grid(AABB *aabb, void *user_data) {
    editor_state.sprite_sheet_grid_y_offset -= 10;
}
void scroll_down_sprite_grid(AABB *aabb, void *user_data) {
    editor_state.sprite_sheet_grid_y_offset += 10;
}

void open_menu_on_toggle(AABB *aabb, void *user_data) {
    vec2 grid_start_position = { global.window.width - 70, global.window.height - 10 };
    float spacing = 2.0f;
    render_sprite_sheet_grid(&global.sprite_sheet_tileset, grid_start_position, spacing); 
    button(
        global.window.width - 45, global.window.height -  20, 40, 10, 4,
        WHITE,
        "Scroll-up",
        scroll_up_sprite_grid,
        NULL
    );
    button(
            global.window.width - 45, 20, 40, 10, 4,
            WHITE,
            "Scroll-down",
            scroll_down_sprite_grid,
            NULL
        );
   
}

void save_level_button(AABB *aabb, void *user_data) {
    save_level_item(editor_state.list_tiled_static_bodies, "./tiled-static-bodies.bin");
    physics_static_body_dump("./static-bodies.bin");
}

void load_level_button(AABB *aabb, void *user_data) {
    load_level();
}

static void editor_ui() {
    button(global.window.width - 45, 50, 40, 10, 4, BLUE, "load level", load_level_button, NULL);
    button(global.window.width - 45, 25, 40, 10, 4, RED, "save level", save_level_button, NULL);
    toggle(global.window.width - 45, 5, 40, 10, 4, GREY, "bla", "open-editor-menu-toggle", open_menu_on_toggle, NULL);
}

void remove_active_body(void) {
    if (editor_state.active_body == (usize)-1) {
        return;
    }

    usize removed_body_index = editor_state.active_body;

    physics_static_body_remove(editor_state.active_body);
    editor_state.active_body = (usize)-1;

    for (usize i = 0; i < editor_state.list_tiled_static_bodies->len;) {
        Tiled_Static_Body *body = array_list_get(editor_state.list_tiled_static_bodies, i);
        if (body->static_body == removed_body_index) {
            array_list_remove(editor_state.list_tiled_static_bodies, i);
        } else {
            if (body->static_body > removed_body_index) {
                body->static_body -= 1;
            }
            i++;
        }
    }
}

void level_editor_render(void) {
    f32 mouseX_world = global.input.mouseX;
    f32 mouseY_world = global.input.mouseY;

    vec2 mousePos_world = {mouseX_world, mouseY_world};


    if (global.input.mouseRightClick) {
        if ((editor_state.action != CREATING) && (editor_state.action != RESIZING)) {
            for (usize i = 0; i < physics_static_body_count(); ++i) {
                Static_Body *static_body = physics_static_body_get(i);

                vec2 size_handle = {5, 5};
                vec2 handle_pos = {
                    static_body->aabb.position[0] + static_body->aabb.half_size[0] - size_handle[0] * 0.5f,
                    static_body->aabb.position[1] - static_body->aabb.half_size[1] + size_handle[1] * 0.5f
                };

                AABB handle_aabb = {
                    .position = {handle_pos[0], handle_pos[1]},
                    .half_size = {size_handle[0] * 0.5f, size_handle[1] * 0.5f}
                };

                if (physics_point_intersect_aabb(mousePos_world, handle_aabb)) {
                    editor_state.active_body = i;
                    editor_state.action = RESIZING;

                    editor_state.startingX = mouseX_world;
                    editor_state.startingY = mouseY_world;

                    editor_state.initial_size[0] = static_body->aabb.half_size[0] * 2.0f;
                    editor_state.initial_size[1] = static_body->aabb.half_size[1] * 2.0f;

                    editor_state.initial_position[0] = static_body->aabb.position[0];
                    editor_state.initial_position[1] = static_body->aabb.position[1];

                    break;
                }
            }

            // If not resizing, start creating a new body as before
            if (editor_state.action != RESIZING) {
                editor_state.action = CREATING;
                editor_state.startingX = mouseX_world;
                editor_state.startingY = mouseY_world;
                vec2 size = {0, 0};
                vec2 position = {editor_state.startingX, editor_state.startingY};
                editor_state.active_body = physics_static_body_create(position, size, COLLISION_LAYER_TERRAIN);
                array_list_append(editor_state.list_tiled_static_bodies, &(Tiled_Static_Body){
                    .tile_coordinates = {
                        .column = editor_state.selected_sprite_coords[0],
                        .row = editor_state.selected_sprite_coords[1],
                    },
                    .static_body = editor_state.active_body
                });
            }
        }
    }

    // Resizing a new body during creation
    if (global.input.mouseRightClick && editor_state.action == CREATING && editor_state.active_body != (usize)-1) {
        vec2 new_position;
        vec2 new_half_size;

        vec2 initial_size = {0, 0};
        vec2 initial_position = {editor_state.startingX, editor_state.startingY};
        vec2 starting_mouse_pos = {editor_state.startingX, editor_state.startingY};
        vec2 current_mouse_pos = {mouseX_world, mouseY_world};

        compute_resized_aabb(initial_position, initial_size, starting_mouse_pos, current_mouse_pos, &new_position, &new_half_size);

        Static_Body *static_body = physics_static_body_get(editor_state.active_body);
        static_body->aabb.position[0] = new_position[0];
        static_body->aabb.position[1] = new_position[1];
        static_body->aabb.half_size[0] = new_half_size[0];
        static_body->aabb.half_size[1] = new_half_size[1];
    }

    // Resizing an existing body
    if (global.input.mouseRightClick && editor_state.action == RESIZING && editor_state.active_body != (usize)-1) {
        vec2 new_position;
        vec2 new_half_size;

        vec2 starting_mouse_pos = {editor_state.startingX, editor_state.startingY};
        vec2 current_mouse_pos = {mouseX_world, mouseY_world};

        compute_resized_aabb(editor_state.initial_position, editor_state.initial_size, starting_mouse_pos, current_mouse_pos, &new_position, &new_half_size);

        Static_Body *static_body = physics_static_body_get(editor_state.active_body);
        static_body->aabb.position[0] = new_position[0];
        static_body->aabb.position[1] = new_position[1];
        static_body->aabb.half_size[0] = new_half_size[0];
        static_body->aabb.half_size[1] = new_half_size[1];
    }

    // Exit creation or resize mode on mouse release
    if (!global.input.mouseRightClick && editor_state.action == CREATING && editor_state.active_body != (usize)-1) {
       editor_state.action = IDLE;
        // editor_state.active_body = (usize)-1;
    }

    if (!global.input.mouseRightClick && editor_state.action == RESIZING && editor_state.active_body != (usize)-1) {
        editor_state.action = IDLE;
        // editor_state.active_body = (usize)-1;
    }

    // Select and move static bodies
    if (global.input.mouseLeftClick && editor_state.action != MOVING) {
        for (usize i = 0; i < physics_static_body_count(); ++i) {
            Static_Body *static_body = physics_static_body_get(i);

            if (physics_point_intersect_aabb(mousePos_world, static_body->aabb)) {
                
                editor_state.active_body = i;
                editor_state.action = MOVING;

                editor_state.offset[0] = static_body->aabb.position[0] - mouseX_world;
                editor_state.offset[1] = static_body->aabb.position[1] - mouseY_world;
                break;
            } else {
                editor_state.active_body = (usize)-1;
            }
        }
    }

    if (global.input.backspace == KS_HELD && editor_state.active_body != (usize)-1) {
        remove_active_body();
    }
    if (global.input.mouseLeftClick && editor_state.action == MOVING && editor_state.active_body != (usize)-1) {
        // Update the position of the active body
        Static_Body *static_body = physics_static_body_get(editor_state.active_body);
        static_body->aabb.position[0] = mouseX_world + editor_state.offset[0];
        static_body->aabb.position[1] = mouseY_world + editor_state.offset[1];
    }

    if (!global.input.mouseLeftClick && editor_state.action == MOVING && editor_state.active_body != (usize)-1) {
        editor_state.action = IDLE;
        // editor_state.active_body = (usize)-1;
    }

    for (u32 i = 0; i < editor_state.list_tiled_static_bodies->len; ++i) {
        Static_Body *static_body = physics_static_body_get(i);
        // Render static bodies with appropriate color or texture

        if (i == editor_state.active_body) {
            if (editor_state.action == CREATING) {
            // While creating, render the AABB with a color
                append_standard_quad((f32 *)static_body, RED);
            }  else if (editor_state.action == RESIZING) {
                append_standard_quad((f32 *)static_body, BLUE);
            } else {
                append_standard_quad((f32 *)static_body, GREEN);
            }
        }         
    
    Tiled_Static_Body *tiled_static_body = array_list_get(editor_state.list_tiled_static_bodies, i);
    render_tiled_static_body(static_body, &global.sprite_sheet_tileset, tiled_static_body->tile_coordinates.row, tiled_static_body->tile_coordinates.column);

    // Render the resize handle
    vec2 size_handle = {5, 5};
    vec2 handle_pos = {
        static_body->aabb.position[0] + static_body->aabb.half_size[0] - size_handle[0] * 0.5f,
        static_body->aabb.position[1] - static_body->aabb.half_size[1] + size_handle[1] * 0.5f
    };
    append_quad_line(handle_pos, size_handle, YELLOW);
    }

    editor_ui();
}

