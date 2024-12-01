#include "ui.h"

#include <stdbool.h>

#include "../render/render.h"
#include "../input/input.h"
#include "../global.h"
#include "../physics/physics.h"

void ui_init(void) {
    const unsigned initial_size = 2;
    if (0 != hashmap_create(initial_size, &global.ui)) {
        ERROR_EXIT("Unable to create ui state hashmap.")
    }
}

bool get_state(const char* id) {
    bool state = false;
    void* const element = hashmap_get(&global.ui, id, strlen(id));
    if (element != NULL) {
        state = (bool)&element;
    } else {
        state = false;
    }
    return state;
}



void toggle(f32 x, f32 y, f32 width, f32 height, u8 border_radius, vec4 color, const char *text, const char *id, On_Toggle on_toggle, void *user_data) {

    char* click_state_id = concat(id, "-click-state");

    bool is_clicked = get_state(click_state_id);

    // Create the AABB for hit detection
    AABB aabb = {
        .position = { x + width * 0.5f, y + height * 0.5f },
        .half_size = { width * 0.5f, height * 0.5f }
    };

    vec2 mouse_pos = { global.input.mouseX, global.input.mouseY };

    bool is_hovered = physics_point_intersect_aabb(mouse_pos, aabb);

    vec4 current_color;
    if (is_hovered) {
        vec4_scale(current_color, color, 0.9f);
    } else {
        memcpy(current_color, color, sizeof(vec4));
    }

    append_rounded_quad(aabb.position, (vec2){ width, height }, current_color, border_radius);

    if (is_hovered && global.input.mouseLeftClick && !is_clicked) {
        if (on_toggle) {
            bool is_toggled = get_state(id);

            if (!is_toggled) {
                bool toggled = true;
                on_toggle(&aabb, user_data);
                if (0 != hashmap_put(&global.ui, id, strlen(id), &toggled)) {
                }
            } else {
                if (0 != hashmap_remove(&global.ui, id, strlen(id))) {
                }
            }

            bool clicked = true;
            if (hashmap_put(&global.ui, click_state_id, strlen(click_state_id), &clicked)) {
            }
        }
    } else {
        if (on_toggle) {
            bool is_toggled = get_state(id);
            if (is_toggled) {
                on_toggle(&aabb, user_data);
            }
        }
    }


    if (!global.input.mouseLeftClick && is_clicked) {
        hashmap_remove(&global.ui, click_state_id, strlen(click_state_id));
        free(click_state_id);
    }
}

void button(f32 x, f32 y, f32 width, f32 height, u8 border_radius, vec4 color, const char *text, On_Click on_click, void *user_data) {
    // Create the AABB for hit detection
    AABB aabb = {
        .position = { x + width * 0.5f, y + height * 0.5f },
        .half_size = { width * 0.5f, height * 0.5f }
    };

    vec2 mouse_pos = { global.input.mouseX, global.input.mouseY };

    bool is_hovered = physics_point_intersect_aabb(mouse_pos, aabb);

    vec4 current_color;
    if (is_hovered) {
        vec4_scale(current_color, color, 0.9f);
    } else {
        memcpy(current_color, color, sizeof(vec4));
    }

    append_rounded_quad(aabb.position, (vec2){ width, height }, current_color, border_radius);

    if (is_hovered && global.input.mouseLeftClick) {
        if (on_click) {
            on_click(&aabb, user_data);
        }
    }
}

void block(f32 x, f32 y, f32 width, f32 height, u8 border_radius, vec4 color) {
    AABB aabb = {
        .position = { x + width * 0.5f, y + height * 0.5f },
        .half_size = { width * 0.5f, height * 0.5f }
    };
    
    append_rounded_quad(aabb.position, (vec2){ width, height }, color, border_radius);
}

