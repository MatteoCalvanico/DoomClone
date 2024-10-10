#ifndef TINYRAYCASTER_H
#define TINYRAYCASTER_H

#include <vector>
#include <SDL.h>

#include "map.h"
#include "player.h"
#include "sprite.h"
#include "framebuffer.h"
#include "textures.h"

struct GameState {
    Map map;
    Player player;
    std::vector<Sprite> monsters;
    Texture tex_walls;
    Texture tex_monst;
    Texture tex_gun;
};

// Render the game state to the framebuffer
void render(FrameBuffer &fb, const GameState &gs, SDL_Renderer* renderer);

#endif // TINYRAYCASTER_H