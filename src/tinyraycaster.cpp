#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <cassert>

#include "../include/headers/utils.h"
#include "../include/headers/tinyraycaster.h"

/**
 * @brief Calculates the texture coordinate for a wall hit in a raycasting engine.
 *
 * This function determines the texture coordinate on the wall where a ray hits,
 * based on the fractional parts of the hit coordinates. It distinguishes between
 * vertical and horizontal walls and ensures the texture coordinate is within valid bounds.
 *
 * @param x The x-coordinate of the hit point.
 * @param y The y-coordinate of the hit point.
 * @param tex_walls Reference to the texture object containing wall textures.
 * @return The texture coordinate corresponding to the hit point.
 */
int wall_x_texcoord(const float hitx, const float hity, const Texture &tex_walls) {
    float x = hitx - floor(hitx + 0.5f); // hitx and hity contain (signed) fractional parts of x and y,
    float y = hity - floor(hity + 0.5f); // they vary between -0.5 and +0.5, and one of them is supposed to be very close to 0
    int tex = static_cast<int>(x * tex_walls.size);

    if (std::abs(y) > std::abs(x)) // we need to determine whether we hit a "vertical" or a "horizontal" wall (w.r.t the map)
        tex = static_cast<int>(y * tex_walls.size);

    if (tex < 0) // fix tax if it is negative
        tex += tex_walls.size;

    assert(tex >= 0 && tex < static_cast<int>(tex_walls.size));

    return tex;
}

/**
 * @brief Draws the map, player, visibility cone, and sprites onto the framebuffer.
 *
 * This function renders the map grid, player position, visibility cone, and sprites
 * onto the provided framebuffer. The map is drawn using the wall textures, and the
 * player and sprites are represented by colored rectangles.
 *
 * @param fb The framebuffer to draw onto.
 * @param sprites A vector of sprites to be drawn on the map.
 * @param tex_walls The texture containing wall textures.
 * @param map The map data structure containing the layout of the map.
 * @param player The player data structure containing the player's position and angle.
 * @param cell_w The width of each cell in the map grid.
 * @param cell_h The height of each cell in the map grid.
 */
void draw_map(FrameBuffer &fb, const std::vector<Sprite> &sprites, const Texture &tex_walls, const Map &map, const Player &player, const size_t cell_w, const size_t cell_h) {
    size_t start_x = fb.w - map.w * cell_w;
    size_t start_y = fb.h - map.h * cell_h;

    for (size_t j = 0; j < map.h; j++) {  // draw the map itself
        
        for (size_t i = 0; i < map.w; i++) {
            size_t rect_x = start_x + i * cell_w;
            size_t rect_y = start_y + j * cell_h;

            if (map.is_empty(i, j)) { // fill the floor
                fb.draw_rectangle(rect_x, rect_y, cell_w, cell_h, tex_walls.get(0, 0, 2));
                continue;
            }

            size_t texid = map.get(i, j);
            assert(texid < tex_walls.count);
            fb.draw_rectangle(rect_x, rect_y, cell_w, cell_h, tex_walls.get(0, 0, texid)); // the color is taken from the upper left pixel of the texture #texid
        }
    }

    // Draw the player on the map
    size_t player_map_x = start_x + player.x * cell_w;
    size_t player_map_y = start_y + player.y * cell_h;
    fb.draw_rectangle(player_map_x, player_map_y, cell_w / 2, cell_h / 2, pack_color(0, 255, 0));

    // !!! Draw the visibility cone here if necessary !!!

    // Draw the sprites on the map
    for (const auto &sprite : sprites) {
        size_t sprite_map_x = start_x + sprite.x * cell_w;
        size_t sprite_map_y = start_y + sprite.y * cell_h;
        fb.draw_rectangle(sprite_map_x, sprite_map_y, cell_w / 2, cell_h / 2, pack_color(255, 0, 0));
    }
}

/**
 * @brief Draws a sprite on the framebuffer.
 *
 * This function renders a sprite on the screen based on the player's position and orientation.
 * It takes into account the depth buffer to handle occlusion and uses a texture to draw the sprite.
 *
 * @param sprite The sprite to be drawn, containing its position and texture ID.
 * @param player The player object, containing the player's position and viewing angle.
 * @param fb The framebuffer where the sprite will be drawn.
 * @param depth_buffer A vector containing depth information for each column of the screen.
 * @param tex_monst The texture object used to draw the sprite.
 */
void draw_sprite(const Sprite &sprite, const Player &player, FrameBuffer &fb, const std::vector<float> &depth_buffer, const Texture &tex_monst) {
    float sprite_dir = atan2(sprite.y - player.y, sprite.x - player.x);
    while (sprite_dir - player.a > M_PI) sprite_dir -= 2 * M_PI;
    while (sprite_dir - player.a < -M_PI) sprite_dir += 2 * M_PI;

    float sprite_dist = sqrt(pow(sprite.x - player.x, 2) + pow(sprite.y - player.y, 2));
    if (sprite_dist > 15) return; // Skip drawing distant sprites

    size_t sprite_screen_size = std::min(1000, static_cast<int>(fb.h / sprite_dist)); // screen sprite size
    int h_offset = (sprite_dir - player.a) * (fb.w) / (player.fov) + (fb.w) / 2 - sprite_screen_size / 2; // full screen width
    int v_offset = fb.h / 2 - sprite_screen_size / 2;

    for (size_t i = 0; i < sprite_screen_size; i++) {
        if (h_offset + int(i) < 0 || h_offset + i >= fb.w) continue;
        if (depth_buffer[h_offset + i] < sprite_dist) continue; // this sprite column is occluded
        for (size_t j = 0; j < sprite_screen_size; j++) {
            if (v_offset + int(j) < 0 || v_offset + j >= fb.h) continue;
            uint32_t color = tex_monst.get(i * tex_monst.size / sprite_screen_size, j * tex_monst.size / sprite_screen_size, sprite.tex_id);
            uint8_t r, g, b, a;
            unpack_color(color, r, g, b, a);
            if (a > 128)
                fb.set_pixel(h_offset + i, v_offset + j, color);
        }
    }
}

/**
 * @brief Renders the game frame, including the 3D view, sprites, and map.
 * 
 * This function clears the framebuffer, performs ray casting to render the 3D view,
 * draws sprites, overlays the map, and displays a prompt if the player is near a door.
 * 
 * @param fb The framebuffer to render to.
 * @param gs The current game state, including the map, player, and sprites.
 * @param renderer The SDL renderer used for drawing text.
 */
void render(FrameBuffer &fb, const GameState &gs, SDL_Renderer* renderer) {
    const Map &map                     = gs.map;
    const Player &player               = gs.player;
    const std::vector<Sprite> &sprites = gs.monsters;
    const Texture &tex_walls           = gs.tex_walls;
    const Texture &tex_monst           = gs.tex_monst;
    const Texture &tex_gun             = gs.tex_gun;

    fb.clear(pack_color(255, 255, 255)); // clear the screen

    const size_t cell_w = fb.w / (map.w * 4); // size of one map cell on the screen
    const size_t cell_h = fb.h / (map.h * 4);

    std::vector<float> depth_buffer(fb.w, 1e3); // buffer to store the Z-coordinate based on the ray casting

    // 3D engine
    for (size_t i = 0; i < fb.w; i++) {
        float angle = player.a - player.fov / 2 + player.fov * i / float(fb.w); // current angle

        // Ray casting - find the distance to the first wall in the specific direction
        for (float t = 0; t < 20; t += .05) { // Increase the increment to reduce iterations
            float x = player.x + t * cos(angle);
            float y = player.y + t * sin(angle);
            if (map.is_empty(x, y) || map.get(x, y) == 9) continue; // ray falls within the screen, but does not touch a wall

            size_t texid = map.get(x, y); // our ray touches a wall, so draw the vertical column to create an illusion of 3D
            assert(texid < tex_walls.count);

            float dist = t * cos(angle - player.a);
            depth_buffer[i] = dist; // save the distance to the wall
            size_t column_height = std::min(2000, int(fb.h / dist));

            int x_texcoord = wall_x_texcoord(x, y, tex_walls);
            std::vector<uint32_t> column = tex_walls.get_scaled_column(texid, x_texcoord, column_height);
            int pix_x = i; // we are drawing at the full width of the screen

            for (size_t j = 0; j < column_height; j++) {
                int pix_y = j + fb.h / 2 - column_height / 2;
                if (pix_y >= 0 && pix_y < int(fb.h)) {
                    fb.set_pixel(pix_x, pix_y, column[j]);
                }
            }
            break;
        }
    }

    // Draw the sprites
    for (const auto &sprite : sprites) {
        draw_sprite(sprite, player, fb, depth_buffer, tex_monst);
    }

    // Draw the map on top of the 3D view
    draw_map(fb, sprites, tex_walls, map, player, cell_w, cell_h);

    // Show gun on the screen
    size_t gun_w = tex_gun.img_w / 2; // Take only the first sprite [No firing animation]
    size_t gun_h = tex_gun.img_h;
    size_t gun_x = (fb.w - gun_w) / 2;
    size_t gun_y = fb.h - gun_h;

    for (size_t y = 0; y < gun_h; y++) {
        for (size_t x = 0; x < gun_w; x++) {
            uint32_t color = tex_gun.get(x, y, 0);    // Get the pixel from the first sprite
            if (color != pack_color(255, 255, 255)) { // Skip the white pixels
                fb.set_pixel(gun_x + x, gun_y + y, color);
            }
        }
    }

    // Check if the player is near a door and show "F to open"
    size_t i = static_cast<size_t>(player.x);
    size_t j = static_cast<size_t>(player.y);
    auto [di, dj] = map.check_door(i, j);
    if (di != 0 || dj != 0) {
        fb.draw_text(renderer, "F to open", fb.w / 2, fb.h - 50, pack_color(255, 255, 0));
    }
}