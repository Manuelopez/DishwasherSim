/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

For a C++ project simply rename the file to .cpp and re-run the build script

-- Copyright (c) 2020-2024 Jeffery Myers
--
--This software is provided "as-is", without any express or implied warranty. In
no event
--will the authors be held liable for any damages arising from the use of this
software.

--Permission is granted to anyone to use this software for any purpose,
including commercial
--applications, and to alter it and redistribute it freely, subject to the
following restrictions:

--  1. The origin of this software must not be misrepresented; you must not
claim that you
--  wrote the original software. If you use this software in a product, an
acknowledgment
--  in the product documentation would be appreciated but is not required.
--
--  2. Altered source versions must be plainly marked as such, and must not be
misrepresented
--  as being the original software.
--
--  3. This notice may not be removed or altered from any source distribution.

*/

#include "assert.h"
#include "raylib.h"
#include "raymath.h"
#include "stdio.h"

#include "range.h"

#include "resource_dir.h" // utility header for SearchAndSetResourceDir
#include <_printf.h>
#include <_stdio.h>
#include <_string.h>
#include <malloc/_malloc.h>
#include <math.h>
#include <stdbool.h>

#define MAX_ENTITY_COUNT 1024
const float entity_selection_radius = 16.0f;

bool almost_equals(float a, float b, float epsilon) {
  return fabs(a - b) <= epsilon;
}
bool animate_f32_to_target(float *value, float target, float delta_t,
                           float rate) {
  *value += (target - *value) * (1.0 - pow(2.0f, -rate * delta_t));
  if (almost_equals(*value, target, 0.001f)) {
    *value = target;

    return true; // reached
  }
  return false;
}

void animate_v2_to_target(Vector2 *value, Vector2 target, float delta_t,
                          float rate) {
  float a = value->x;
  animate_f32_to_target(&(value->x), target.x, delta_t, rate);

  float b = value->x;

  animate_f32_to_target(&(value->y), target.y, delta_t, rate);
}

const int tile_width = 8;
int world_pos_to_tile_pos(float world_pos) {
  return roundf(world_pos / (float)tile_width);
}
int tile_pos_to_world_pos(int tile_pos) {
  return (float)tile_pos * (float)tile_width;
}

Vector2 round_v2_to_tile(Vector2 world_pos) {
  world_pos.x = tile_pos_to_world_pos(world_pos_to_tile_pos(world_pos.x));
  world_pos.y = tile_pos_to_world_pos(world_pos_to_tile_pos(world_pos.y));
  return world_pos;
}

typedef enum EntityArcheType {
  nil = 0,
  arch_troll = 1,
  arch_goblin = 2,
  arch_player = 3,
  arch_card_fireball = 4

} EntityArcheType;

typedef struct Sprite {
  Texture image;
  Vector2 size;
} Sprite;

typedef enum SpriteId {
  SPRITE_NIL,
  SPRITE_PLAYER,
  SPRITE_GOBLIN,
  SPRITE_TROLL,
  SPRITE_CARD_FIREBALL,
  SPRITE_MAX
} SpriteId;

typedef struct Entity {
  Vector2 pos;
  bool is_valid;
  Texture *sprite;
  EntityArcheType type;
  SpriteId sprite_id;
} Entity;

// maybe make this into an x macro
typedef struct World {
  Entity entities[MAX_ENTITY_COUNT];
} World;

typedef struct WorldFrame {
  Entity *selected_entity;

} WorldFrame;
WorldFrame world_frame = {0};

World *world = 0;

Entity *create_entity() {
  Entity *entity_found = 0;

  for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
    Entity *existing_entity = &world->entities[i];
    if (!existing_entity->is_valid) {
      entity_found = existing_entity;
      break;
      printf("%i - times", i);
    }
  }

  //  assert(entity_found, "No more free entities");
  assert(entity_found);

  entity_found->is_valid = true;
  return entity_found;
}

void destroy_entity(Entity *en) { memset(en, 0, sizeof(Entity)); }
void setup_troll(Entity *en) {
  en->type = arch_troll;
  en->sprite_id = SPRITE_TROLL;
  //...
}

void setup_player(Entity *en) {
  en->type = arch_player;
  en->sprite_id = SPRITE_PLAYER;
  //...
}

void setup_goblin(Entity *en) {
  en->type = arch_goblin;
  en->sprite_id = SPRITE_GOBLIN;
  //...
}

Sprite sprites[SPRITE_MAX];
Sprite *get_sprite(SpriteId id) {
  if (id >= 0 && id < SPRITE_MAX) {
    return &sprites[id];
  }

  return &sprites[0];
}

int main() {
  // init window
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  const int screen_width = 800;
  const int screen_height = 450;
  InitWindow(screen_width, screen_height, "Hello Raylib");

  // set up resources folder
  SearchAndSetResourceDir("resources");

  // set up golbal variables

  world = (World *)malloc(sizeof(World));
  if (world == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }
  for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
    world->entities[i] = (Entity){.is_valid = false};
  }
  sprites[SPRITE_PLAYER] =
      (Sprite){.image = LoadTexture("player.png"), .size = (Vector2){10, 19}};
  sprites[SPRITE_TROLL] =
      (Sprite){.image = LoadTexture("troll.png"), .size = (Vector2){8, 14}};
  sprites[SPRITE_GOBLIN] =
      (Sprite){.image = LoadTexture("goblin.png"), .size = (Vector2){8, 14}};

  // entities
  for (int i = 0; i < 2; i++) {
    Entity *en = create_entity();
    setup_troll(en);
    Entity *en2 = create_entity();
    setup_goblin(en2);
    en->pos = (Vector2){GetRandomValue(0, 200), GetRandomValue(0, 200)};
    en->pos = round_v2_to_tile(en->pos);

    en2->pos = (Vector2){GetRandomValue(0, 200), GetRandomValue(0, 200)};
    en2->pos = round_v2_to_tile(en2->pos);
  }
  Entity *player_entity = create_entity();
  setup_player(player_entity);
  assert(player_entity);

  player_entity->pos = (Vector2){0, 0};

  // camera
  Camera2D camera = {0};
  camera.zoom = 3.0f;
  camera.offset = (Vector2){screen_width / 2.0f, screen_height / 2.0f};
  camera.rotation = 0;
  camera.target = (Vector2){
      player_entity->pos.x + (sprites[SPRITE_PLAYER].image.width / 2.0f),
      player_entity->pos.y + (sprites[SPRITE_PLAYER].image.height / 2.0f)};

  //  game loop
  while (!WindowShouldClose()) // run the loop untill the user presses ESCAPE or
  {

    world_frame = (WorldFrame){0};
    // update player position
    Vector2 input_axis = {0, 0};

    if (IsKeyDown(KEY_W)) {
      input_axis.y -= 1;
    }
    if (IsKeyDown(KEY_A)) {

      input_axis.x -= 1;
    }
    if (IsKeyDown(KEY_S)) {

      input_axis.y += 1;
    }
    if (IsKeyDown(KEY_D)) {

      input_axis.x += 1;
    }

    input_axis = Vector2Normalize(input_axis);
    player_entity->pos = Vector2Add(
        player_entity->pos, Vector2Scale(input_axis, 100.0 * GetFrameTime()));

    // camera stuff
    {

      Vector2 target = player_entity->pos;
      target.x = target.x + (sprites[SPRITE_PLAYER].image.width) / 2.0f;
      target.y = target.y + (sprites[SPRITE_PLAYER].image.height) / 2.0f;
      /* camera.target = (Vector2){ */
      /*     player_entity->pos.x + (sprites[SPRITE_PLAYER].image.width) / 2.0f,
       */
      /*     player_entity->pos.y + (sprites[SPRITE_PLAYER].image.height)
       * / 2.0f}; */
      animate_v2_to_target(&camera.target, target, GetFrameTime(), 3.0f);
    }

    // begin drawind
    BeginDrawing();

    ClearBackground(LIGHTGRAY);

    // 2d world
    {
      BeginMode2D(camera);

      Vector2 mouse_pos_world = GetScreenToWorld2D(GetMousePosition(), camera);
      printf("(%.2f - %.2f) \n", mouse_pos_world.x, mouse_pos_world.y);
      int mouse_tile_x = world_pos_to_tile_pos(mouse_pos_world.x);
      int mouse_tile_y = world_pos_to_tile_pos(mouse_pos_world.y);

      // :tile rendering
      {
        int player_tile_x = world_pos_to_tile_pos(player_entity->pos.x);
        int player_tile_y = world_pos_to_tile_pos(player_entity->pos.y);

        const int tile_radius_x = 40;
        const int tile_radius_y = 30;

        for (int x = player_tile_x - tile_radius_x;
             x < player_tile_x + tile_radius_x; x++) {
          for (int y = player_tile_y - tile_radius_y;
               y < player_tile_y + tile_radius_y; y++) {
            if ((x + (y % 2 == 0)) % 2 == 0) {
              float x_pos = x * tile_width;
              float y_pos = y * tile_width;
              Color tile_color = WHITE;
              DrawRectangle(x_pos + (tile_width * -0.5),
                            y_pos + (tile_width * -0.5), tile_width, tile_width,
                            tile_color);
            }
          }
        }

        /* DrawRectangle(tile_pos_to_world_pos(mouse_tile_x) + (tile_width *
         * -0.5), */
        /*               tile_pos_to_world_pos(mouse_tile_y) + (tile_width *
         * -0.5), */
        /*               tile_width, tile_width, BLUE); */
      }

      // :mouse pos tester
      {
        float smallest_dist = INFINITY;

        for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
          Entity *en = &world->entities[i];
          if (en->is_valid) {
            Sprite *sprite = get_sprite(en->sprite_id);

            float dist = fabsf(Vector2Distance(en->pos, mouse_pos_world));
            if (dist < entity_selection_radius) {

              if (!world_frame.selected_entity || (dist < smallest_dist)) {
                world_frame.selected_entity = en;
                smallest_dist = dist;
              }

              /* int entity_tile_x = world_pos_to_tile_pos(en->pos.x); */
              /* int entity_tile_y = world_pos_to_tile_pos(en->pos.y); */

              /* DrawRectangle(tile_pos_to_world_pos(entity_tile_x), */
              /*               tile_pos_to_world_pos(entity_tile_y), tile_width,
               */
              /*               tile_width, BLUE); */
            }
            /* Range2 bounds = range2_make_bottom_happen(sprite->size); */
            /* bounds = range2_shift(bounds, en->pos); */
            /* bounds.min = Vector2Subtract(bounds.min,
             * (Vector2){10.0f, 10.0f}); */
            /* bounds.max = Vector2Add(bounds.max, (Vector2){10.0f, 10.0f}); */
            /* Vector2 rect_size = range2_size(bounds); */
            /* if (range2_contains(bounds, mouse_pos)) { */
            /*   DrawRectangle(bounds.min.x, bounds.min.y, rect_size.x, */
            /*                 rect_size.y, RED); */
            /* } */
          }
        }
      }

      // :render
      {
        for (int i = 0; i < MAX_ENTITY_COUNT; i++) {
          Entity *en = &world->entities[i];
          if (en->is_valid) {
            switch (en->type) {
            default: {
              Sprite *sprite = get_sprite(en->sprite_id);
              Color entity_color = WHITE;
              if (world_frame.selected_entity == en) {
                entity_color = RED;
              }
              DrawTexture(sprite->image, en->pos.x - (sprite->size.x / 2.0f),
                          en->pos.y - (sprite->size.y / 2.0f), entity_color);

              /* char pos_string[100]; */
              /* sprintf(pos_string, "(%.2f - %.2f)", en->pos.x, en->pos.y); */
              /* DrawText(pos_string, en->pos.x + 10, en->pos.y + 10, 5, BLACK);
               */
              break;
            }
            }
          }
        }
      }

      EndMode2D();
    }

    // end the frame and get ready for the next one  (display frame, poll input,
    // etc...)
    EndDrawing();
  }

  // cleanup
  // unload our texture so it can be cleaned up
  UnloadTexture(sprites[SPRITE_TROLL].image);
  UnloadTexture(sprites[SPRITE_PLAYER].image);
  UnloadTexture(sprites[SPRITE_GOBLIN].image);

  // destory the window and cleanup the OpenGL context
  CloseWindow();
  return 0;
}
