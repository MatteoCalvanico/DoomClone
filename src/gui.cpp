#define SDL_MAIN_HANDLED

#include <iostream>
#include <vector>
#include <math.h>

#include <SDL.h>
#include "../include/sdl/SDL.h"

#include "../include/headers/map.h"
#include "../include/headers/utils.h"
#include "../include/headers/player.h"
#include "../include/headers/sprite.h"
#include "../include/headers/framebuffer.h"
#include "../include/headers/textures.h"
#include "../include/headers/tinyraycaster.h"

/**
 * @brief Entry point of the application.
 * 
 * Initializes the framebuffer, player, map, and textures. Renders the initial scene and sets up the SDL window and renderer.
 * Enters the main event loop to handle SDL events and render the framebuffer to the window.
 * 
 * @return int Returns 0 on successful execution, -1 on failure to load textures or initialize SDL.
 * 
 * The function performs the following steps:
 * 1. Initializes the framebuffer with a white background.
 * 2. Initializes the player with a starting position and direction.
 * 3. Loads the map.
 * 4. Loads the wall and monster textures.
 * 5. Checks if the textures are loaded successfully.
 * 6. Initializes a list of sprites.
 * 7. Renders the initial scene.
 * 8. Initializes SDL and creates a window and renderer.
 * 9. Creates an SDL texture for the framebuffer.
 * 10. Enters the main event loop to handle SDL events and render the framebuffer.
 * 11. Cleans up SDL resources before exiting.
 */
int main() {
    FrameBuffer fb{1024, 512, std::vector<uint32_t>(1024*512, pack_color(255, 255, 255))};
    Player player{2, 14, 270, M_PI/3., 0, 0};
    std::vector<Sprite> sprites{ {4, 14, 0, 0}, {6, 14.50, 1, 0}, {8, 13.50, 2, 0} };
    Map map;

    Texture tex_walls("texture/walltext.png");
    Texture tex_monst("texture/monsters.png");
    if (!tex_walls.count || !tex_monst.count) {
        std::cerr << "Failed to load textures" << std::endl;
        return -1;
    }
    
    //render(fb, map, player, sprites, tex_walls, tex_monst);

    SDL_Window   *window   = nullptr;
    SDL_Renderer *renderer = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Couldn't initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }
    if (SDL_CreateWindowAndRenderer(fb.w, fb.h, SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS, &window, &renderer)) {
        std::cerr << "Couldn't create window and renderer: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Texture *framebuffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, fb.w, fb.h);
    //SDL_UpdateTexture(framebuffer_texture, NULL, reinterpret_cast<void *>(fb.img.data()), fb.w*4);
    
    SDL_Event event;
    while (1) {
        // Handle events - player movement and window close
        if (SDL_PollEvent(&event)) {
            if (SDL_QUIT==event.type || (SDL_KEYDOWN==event.type && SDLK_ESCAPE==event.key.keysym.sym)) break;
            if (SDL_KEYUP==event.type) {
                if ('a'==event.key.keysym.sym || 'd'==event.key.keysym.sym) player.turn = 0;
                if ('w'==event.key.keysym.sym || 's'==event.key.keysym.sym) player.walk = 0;
            }
            if (SDL_KEYDOWN==event.type) {
                if ('a'==event.key.keysym.sym) player.turn = -1;
                if ('d'==event.key.keysym.sym) player.turn =  1;
                if ('w'==event.key.keysym.sym) player.walk =  1;
                if ('s'==event.key.keysym.sym) player.walk = -1;
            }
        }

        player.a += float(player.turn)*.05;
        float nx = player.x + player.walk*cos(player.a)*.1;
        float ny = player.y + player.walk*sin(player.a)*.1;
        if (int(nx)>=0 && int(nx)<int(map.w) && int(ny)>=0 && int(ny)<int(map.h) && map.is_empty(nx, ny)) {
            player.x = nx;
            player.y = ny;
        }
        render(fb, map, player, sprites, tex_walls, tex_monst);
        SDL_UpdateTexture(framebuffer_texture, NULL, reinterpret_cast<void *>(fb.img.data()), fb.w*4);
        
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, framebuffer_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(framebuffer_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}