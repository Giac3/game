#include <stdio.h>
#include <stdbool.h>
#include <glad/glad.h>

#define SLD_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "engine/global.h"
#include "engine/config/config.h"
#include "engine/input/input.h"
#include "engine/time/time.h"
#include "engine/physics/physics.h"
#include "engine/util/util.h"
#include "engine/entity/entity.h"
#include "engine/render/render.h"
#include "engine/animation/animation.h"
#include "engine/audio/audio.h"
#include "engine/editor/editor.h"
#include "engine/ui/ui.h"

static AudioMusic MUSIC_STAGE_1;
static AudioSound SOUND_JUMP;

static const f32 SPEED_ENEMY_LARGE = 200;
static const f32 SPEED_ENEMY_SMALL = 400;
static const f32 HEALTH_ENEMY_LARGE = 7;
static const f32 HEALTH_ENEMY_SMALL = 3;


static bool should_quit = false;
vec4 player_color = {0, 1, 1, 1};
bool player_is_grounded = false;


static void input_handle(Body *body_player) {
    if (global.input.escape > 0) {
        should_quit = true;
    };

    f32 velx = 0;
    f32 vely = body_player->velocity[1];

    if (global.input.right > 0) {
        velx += 400;
    };
    if (global.input.left > 0) {
        velx -= 400;
    };
    if (global.input.up && player_is_grounded) {
        player_is_grounded = false;
        vely = 2000;
        audio_sound_play(&SOUND_JUMP);
    };
    
    body_player->velocity[0] = velx;
    body_player->velocity[1] = vely;
};


void player_on_hit(Body *self, Body *other, Hit hit) {
    if (other->collision_layer == COLLISION_LAYER_ENEMY) {
        player_color[0] = 1;
        player_color[2] = 0;
    }
}

void player_on_hit_static(Body *self, Static_Body *other, Hit hit) {
    if (hit.normal[1] > 0) {
        player_is_grounded = true;
    };

    if (hit.normal[0] > 0) {
        self->velocity[0] = 400;
    };
    if (hit.normal[0] < 0) {
        self->velocity[0] = -400;
    };
}

void enemy_on_hit(Body *self, Body *other, Hit hit) {
    if (hit.normal[0] > 0) {
        self->velocity[0] = 200 ;
    };
    if (hit.normal[0] < 0) {
        self->velocity[0] = -200;
    };
}

void enemy_small_on_hit_static(Body *self, Static_Body *other, Hit hit) {
    if (hit.normal[0] > 0) {
        self->velocity[0] = SPEED_ENEMY_SMALL;
    };
    if (hit.normal[0] < 0) {
        self->velocity[0] = -SPEED_ENEMY_SMALL;
    };
}
void enemy_large_on_hit_static(Body *self, Static_Body *other, Hit hit) {
    if (hit.normal[0] > 0) {
        self->velocity[0] = SPEED_ENEMY_LARGE;
    };
    if (hit.normal[0] < 0) {
        self->velocity[0] = -SPEED_ENEMY_LARGE;
    };
}

void fire_on_hit(Body *self, Body * other, Hit hit) {
    if (other->collision_layer == COLLISION_LAYER_ENEMY) {
        for (usize i = 0; i < entity_count(); ++i) {
            Entity *entity = entity_get(i);

            if (entity->body_id == hit.other_id) {
                Body *body = physics_body_get(entity->body_id);
                body->is_active = false;
                entity->is_active = false;
                break;
            }
        }
    }
}

// void render_sprite_sheet_grid(Sprite_Sheet *sprite_sheet, vec2 start_position, float spacing) {
//     int total_columns = (int)(sprite_sheet->width / sprite_sheet->cell_width);
//     int total_rows = (int)(sprite_sheet->height / sprite_sheet->cell_height);

//     for (int row = 0; row < total_rows; ++row) {
//         for (int column = 0; column < total_columns; ++column) {
//             // Calculate the position to render this tile
//             float x = start_position[0] + column * (sprite_sheet->cell_width + spacing);
//             float y = start_position[1] + row * (sprite_sheet->cell_height + spacing);

//             vec2 position = { x, y };

//             // Render the tile at the calculated position
//             render_sprite_sheet_frame(sprite_sheet, row, column, position, false);

//             // Optionally, render a border around each tile
//             vec2 tile_size = { sprite_sheet->cell_width, sprite_sheet->cell_height };
//             render_quad(position, tile_size, WHITE);
//         }
//     }
// }

int main() {
    time_init(60);
    config_init();
    SDL_Window *window = render_init();
    physics_init();
    entity_init();
    ui_init();
    animation_init();
    audio_init();
    editor_init();

    audio_sound_load(&SOUND_JUMP, "./assets/jump.wav");
    // audio_music_load(&MUSIC_STAGE_1, "./assets/song.mp3");
    
    audio_music_play(&MUSIC_STAGE_1);

    SDL_ShowCursor(false);

    u8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
    u8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
    u8 fire_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_PLAYER;

    u32 player_id = entity_create((vec2){200, 100}, (vec2){24, 24}, (vec2){0,0}, 0.4, COLLISION_LAYER_PLAYER, player_mask, false, player_on_hit, player_on_hit_static);

    f32 render_scale = render_get_scale();

    i32 window_width, window_height; 
    SDL_GetWindowSize(window, &window_width, &window_height);
    f32 width = window_width / render_scale; 
    f32 height = window_height / render_scale;

    global.window.width = width;
    global.window.height = height;

    // u32 static_body_a_id = physics_static_body_create((vec2){width * 0.5, height}, (vec2){width, 50}, COLLISION_LAYER_TERRAIN);
    // u32 static_body_b_id = physics_static_body_create((vec2){width - 12.5, height * 0.5}, (vec2){25, height}, COLLISION_LAYER_TERRAIN);
    // u32 static_body_c_id = physics_static_body_create((vec2){width * 0.5, 12.5}, (vec2){width - 50, 50}, COLLISION_LAYER_TERRAIN);
    // u32 static_body_d_id = physics_static_body_create((vec2){12.5, height * 0.5 - 12.5}, (vec2){25, height - 25}, COLLISION_LAYER_TERRAIN);
    // u32 static_body_e_id = physics_static_body_create((vec2){width * 0.5, height * 0.5}, (vec2){62.5, 62.5}, COLLISION_LAYER_TERRAIN);

    usize entity_fire = entity_create((vec2){370, 50}, (vec2){25, 25}, (vec2){0}, 1 , 0, fire_mask, true, fire_on_hit, NULL);

    // usize entity_a_id = entity_create((vec2){200, 100}, (vec2){25, 25}, (vec2){900, 0}, 0.4, COLLISION_LAYER_ENEMY, enemy_mask, NULL, enemy_on_hit_static);
    // usize entity_b_id = entity_create((vec2){300, 100}, (vec2){25, 25}, (vec2){900, 0}, 0.4, 0, enemy_mask, NULL, enemy_on_hit_static);

    Sprite_Sheet sprite_sheet_player;

    render_sprite_sheet_init(&sprite_sheet_player, "./assets/player.png", 24, 24);

    Sprite_Sheet sprite_sheet_tileset;

    render_sprite_sheet_init(&sprite_sheet_tileset, "./assets/pack/tileset.png", 8, 8);

    global.sprite_sheet_tileset = sprite_sheet_tileset;


    usize adef_player_walk_id = animation_definition_create(
        &sprite_sheet_player,
        (f32[]){0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1},
        (u8[]){0, 0, 0, 0, 0, 0, 0},
        (u8[]){1, 2, 3, 4, 5, 6, 7},
        7
    );

    usize adef_player_idle_id = animation_definition_create(&sprite_sheet_player, (f32[]){0}, (u8[]){0}, (u8[]){0}, 1);
    usize anim_player_walk_id = animation_create(adef_player_walk_id, true);
    usize anim_player_idle_id = animation_create(adef_player_idle_id, false);

    Entity *player = entity_get(player_id);

    player->animation_id = anim_player_idle_id;

    f32 spawn_timer = 0;

    int mouseX_window, mouseY_window;
    
    while (!should_quit) {
        time_update();
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type){
            case SDL_QUIT:
                should_quit = true;
                break;
            case SDL_MOUSEWHEEL:
                break;
            case SDL_MOUSEMOTION:                
                SDL_GetMouseState(&mouseX_window, &mouseY_window);
                // Convert to world coordinates
                global.input.mouseX = mouseX_window / render_scale;
                global.input.mouseY = (window_height - mouseY_window) / render_scale;
                break;
            case SDL_MOUSEBUTTONDOWN: 
                if (event.button.button==SDL_BUTTON_LEFT) {
                    global.input.mouseLeftClick = true;    
                };
                if (event.button.button==SDL_BUTTON_RIGHT) {
                    global.input.mouseRightClick = true;    
                };
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button==SDL_BUTTON_LEFT) {
                    global.input.mouseLeftClick = false;    
                };
                if (event.button.button==SDL_BUTTON_RIGHT) {
                    global.input.mouseRightClick = false;    
                }
                break;
            default:
                break;
            }
        }

        // Entity *player = entity_get(player_id);
        // Body *body_player = physics_body_get(player->body_id);

        // if (body_player->velocity[0] != 0) {
        //     player->animation_id = anim_player_walk_id;
        // } else {
        //     player->animation_id = anim_player_idle_id;
        // }

        // Static_Body *static_body_a = physics_static_body_get(static_body_a_id);
        // Static_Body *static_body_b = physics_static_body_get(static_body_b_id);
        // Static_Body *static_body_c = physics_static_body_get(static_body_c_id);
        // Static_Body *static_body_d = physics_static_body_get(static_body_d_id);
        // Static_Body *static_body_e = physics_static_body_get(static_body_e_id);

        input_update();
        // input_handle(body_player);
        physics_update();

        animation_update(global.time.delta);

        // Spawn enemies.

        // {
        //     spawn_timer -= global.time.delta;
        //     if (spawn_timer <= 0) {
        //         spawn_timer = (f32)((rand() % 200) + 200) / 100.f;
        //         spawn_timer *= 0.2;

        //         for (u32 i = 0; i < 10; ++i) {                
        //             f32 spawn_x =  100;

        //             usize entity_id = entity_create(
        //                 (vec2){spawn_x, 100},
        //                 (vec2){20, 20},
        //                 (vec2){0, 0},
        //                 1,
        //                 COLLISION_LAYER_ENEMY,
        //                 enemy_mask,
        //                 false,
        //                 NULL,
        //                 enemy_small_on_hit_static
        //             );
        //             Entity *entity = entity_get(entity_id);
        //             Body *body = physics_body_get(entity->body_id);
        //             float speed = SPEED_ENEMY_SMALL * ((rand() % 100) * 0.01) + 100;
        //             body->velocity[0] = speed;
        //         }
        //     }
        // }
        
        render_begin();

        level_editor_render();
        

        // for (usize i = 0; i < entity_count(); ++i) {
        //     Entity *entity = entity_get(i);
        //     Body *body = physics_body_get(entity->body_id);

        //     if (body->is_active) {
        //         render_aabb((f32*)body, TURQUOISE);
        //     } else {
        //         render_aabb((f32*)body, RED);
        //     }
        // }

        
        // render_aabb((f32 *)static_body_b, WHITE);
        // render_aabb((f32 *)static_body_c, WHITE);
        // render_aabb((f32 *)static_body_d, WHITE);
        // render_aabb((f32 *)static_body_e, WHITE);
        // render_aabb((f32 *)body_player, player_color);

        
        

        // for (usize i = 0; i < entity_count(); ++i) {
        //     Entity *entity = entity_get(i);
        //     if (!entity->is_active) {
        //         continue;
        //     };
        //     if (entity->animation_id == (usize)-1) {
        //         continue;
        //     };

        //     Body *body = physics_body_get(entity->body_id);
        //     Animation *anim = animation_get(entity->animation_id);
        //     Animation_Definition *adef = anim->definition;
        //     Animation_Frame *aframe = &adef->frames[anim->current_frame_index];

        //     if (body->velocity[0] < 0) {
        //         anim->is_flipped = true;
        //     } else if (body->velocity[0]) {
        //         anim->is_flipped = false;
        //     };

        //     render_sprite_sheet_frame(adef->sprite_sheet, aframe->row, aframe->column, body->aabb.position, anim->is_flipped);
        // }

        
        // render_sprite_sheet_frame(&sprite_sheet_player, 0, 4, (vec2){200, 200}, false);
        // render_sprite_sheet_frame(&sprite_sheet_player, 0, 4, body_player->aabb.position);
        // render_sprite_sheet_frame(&sprite_sheet_tileset, 0, 0, (vec2){50, 50}, false);
        // render_sprite_sheet_frame(&sprite_sheet_tileset, 1, 0, (vec2){55, 55}, false);
        // render_sprite_sheet_frame(&sprite_sheet_tileset, 0, 1, (vec2){60, 60}, false);
        // render_sprite_sheet_frame(&sprite_sheet_tileset, 0, 1, (vec2){60, 50}, false);
        // render_sprite_sheet_frame(&sprite_sheet_tileset, 19, 2, (vec2){65, 65}, false);

        // vec2 grid_start_position = { 10.0f, 10.0f }; // Adjust as needed
        // float spacing = 2.0f; // Adjust as needed
        // render_sprite_sheet_grid(&sprite_sheet_tileset, grid_start_position, spacing);

        render_end(window);
        player_color[0] = 0;
        player_color[2] = 1;
        time_update_late();
    }
    
    return 0;
}