#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <iomanip>

#include "../include/headers/map.h"
#include "../include/headers/utils.h"
#include "../include/headers/player.h"
#include "../include/headers/sprite.h"
#include "../include/headers/framebuffer.h"
#include "../include/headers/textures.h"

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
int wall_x_texcoord(const float hitx, const float hity, Texture &tex_walls) {
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
 * @brief Draws a sprite on the map within the framebuffer.
 *
 * This function calculates the position of the sprite on the screen based on the map dimensions
 * and draws a small rectangle representing the sprite.
 *
 * @param sprite The sprite to be drawn, containing its position.
 * @param fb The framebuffer where the sprite will be drawn.
 * @param map The map containing the dimensions used for calculating the sprite's position.
 */
void map_show_sprite(Sprite &sprite, FrameBuffer &fb, Map &map) {
    const size_t rect_w = fb.w/(map.w*2); // size of one map cell on the screen
    const size_t rect_h = fb.h/map.h;

    fb.draw_rectangle(sprite.x*rect_w-3, sprite.y*rect_h-3, 6, 6, pack_color(255, 0, 0));
}

/**
 * @brief Draws a sprite on the framebuffer.
 *
 * This function renders a sprite onto the framebuffer, taking into account the player's position,
 * direction, and the depth buffer to handle occlusion. The sprite is scaled based on its distance
 * from the player and is drawn only if it is within the player's field of view and not occluded by
 * other objects.
 *
 * @param sprite The sprite to be drawn, containing its position and texture ID.
 * @param depth_buffer A vector containing depth information for each column of the framebuffer.
 * @param fb The framebuffer where the sprite will be drawn.
 * @param player The player object, containing the player's position and viewing angle.
 * @param tex_sprites The texture containing the sprite's image.
 */
void draw_sprite(Sprite &sprite, std::vector<float> &depth_buffer, FrameBuffer &fb, Player &player, Texture &tex_sprites) {
    // absolute direction from the player to the sprite (in radians)
    float sprite_dir = atan2(sprite.y - player.y, sprite.x - player.x);
    while (sprite_dir - player.a >  M_PI) sprite_dir -= 2*M_PI; // remove unncesessary periods from the relative direction
    while (sprite_dir - player.a < -M_PI) sprite_dir += 2*M_PI;

    size_t sprite_screen_size = std::min(1000, static_cast<int>(fb.h/sprite.player_dist)); // screen sprite size
    int h_offset = (sprite_dir - player.a)*(fb.w/2) + (fb.w/2)/2 - tex_sprites.size/2; // do not forget the 3D view takes only a half of the framebuffer
    int v_offset = fb.h/2 - sprite_screen_size/2;

    for (size_t i=0; i<sprite_screen_size; i++) {
        if (h_offset+int(i)<0 || h_offset+i>=fb.w/2) continue;
        if (depth_buffer[h_offset+i]<sprite.player_dist) continue; // this sprite column is occluded
        for (size_t j=0; j<sprite_screen_size; j++) {
            if (v_offset+int(j)<0 || v_offset+j>=fb.h) continue;
            uint32_t color = tex_sprites.get(i*tex_sprites.size/sprite_screen_size, j*tex_sprites.size/sprite_screen_size, sprite.tex_id);
            uint8_t r,g,b,a;
            unpack_color(color, r, g, b, a);
            if (a>128)
            fb.set_pixel(fb.w/2 + h_offset+i, v_offset+j, color);
        }
    }
}

/**
 * @brief Renders the game frame, including the map, player visibility cone, 3D view, and sprites.
 * 
 * @param fb The framebuffer to draw on.
 * @param map The game map containing walls and empty spaces.
 * @param player The player object containing position and viewing angle.
 * @param sprites A vector of sprites to be rendered on the map.
 * @param tex_walls Texture object containing wall textures.
 * @param tex_monst Texture object containing monster textures.
 * 
 * This function performs the following tasks:
 * - Clears the framebuffer.
 * - Draws the map by filling each cell with the corresponding wall texture.
 * - Draws the player's visibility cone.
 * - Performs ray casting to determine the distance to walls and renders the 3D view.
 * - Draws the sprites on the map.
 */
void render(FrameBuffer &fb, Map &map, Player &player, std::vector<Sprite> &sprites, Texture &tex_walls, Texture &tex_monst) {
    fb.clear(pack_color(255, 255, 255)); // clear the screen

    const size_t rect_w = fb.w/(map.w*2); // size of one map cell on the screen
    const size_t rect_h = fb.h/map.h;

    // Draw the map - fill each cell with the corresponding texture
    for (size_t j=0; j<map.h; j++) { 
        for (size_t i=0; i<map.w; i++) {
            
            // fill the floor
            if (map.is_empty(i, j)){

                size_t rect_x = i*rect_w;
                size_t rect_y = j*rect_h;
                fb.draw_rectangle(rect_x, rect_y, rect_w, rect_h, tex_walls.get(0, 0, 2));

                continue;
            }

            size_t rect_x = i*rect_w;
            size_t rect_y = j*rect_h;
            size_t texid = map.get(i, j);

            assert(texid<tex_walls.count);

            // Fill the wall with the color from the upper left pixel of the texture #texid
            fb.draw_rectangle(rect_x, rect_y, rect_w, rect_h, tex_walls.get(0, 0, texid));
        }
    }

    std::vector<float> depth_buffer(fb.w/2, 1e3); // buffer to store the Z-coordinate based on the ray casting

    // Draw the visibility cone AND the 3D view
    for (size_t i=0; i<fb.w/2; i++) { 

        float angle = player.a-player.fov/2 + player.fov*i/float(fb.w/2); // current angle

        // Ray casting - find the distance to the first wall in the specific direction
        for (float t=0; t<20; t+=.01) {
            float x = player.x + t*cos(angle);
            float y = player.y + t*sin(angle);
            fb.set_pixel(x*rect_w, y*rect_h, pack_color(160, 160, 160)); // this draws the visibility cone

            if (map.is_empty(x, y)) continue; // ray falls within the screen, but does not touch a wall

            size_t texid = map.get(x, y); // our ray touches a wall, so draw the vertical column to create an illusion of 3D
            assert(texid<tex_walls.count);

            float dist = .2+t*cos(angle-player.a);
            depth_buffer[i] = dist; // save the distance to the wall
            size_t column_height = fb.h/dist;

            int x_texcoord = wall_x_texcoord(x, y, tex_walls);
            std::vector<uint32_t> column = tex_walls.get_scaled_column(texid, x_texcoord, column_height);
            int pix_x = i + fb.w/2; // we are drawing at the right half of the screen, thus +fb.w/2

            for (size_t j=0; j<column_height; j++) { // copy the texture column to the framebuffer
                int pix_y = j + fb.h/2 - column_height/2;
                if (pix_y>=0 && pix_y<(int)fb.h) {
                    fb.set_pixel(pix_x, pix_y, column[j]);
                }
            }
            break;
        }
    }

    // Update the distances from the player to each sprite
    for (size_t i=0; i<sprites.size(); i++) { 
        sprites[i].player_dist = std::sqrt(pow(player.x - sprites[i].x, 2) + pow(player.y - sprites[i].y, 2));
    }

    // Sort the sprites from farthest to closest
    std::sort(sprites.begin(), sprites.end()); 

    // Draw the sprites
    for (size_t i=0; i<sprites.size(); i++) { 
        map_show_sprite(sprites[i], fb, map);
        draw_sprite(sprites[i], depth_buffer, fb, player, tex_monst);
    }
}

/*
int main() {

    // Create a framebuffer, player, map, and wall textures
    FrameBuffer fb{1024, 512, std::vector<uint32_t>(1024*512, pack_color(255, 255, 255))};

    Player player{2, 14, 270, M_PI/3.};

    Map map;
    Texture tex_walls("texture/walltext.png");
    Texture tex_monst("texture/monsters.png");
    if (!tex_walls.count || !tex_monst.count) {
        std::cerr << "Failed to load textures" << std::endl;
        return -1;
    }
    std::vector<Sprite> sprites{ {4, 14, 0, 0}, {6, 14.50, 1, 0}, {8, 13.50, 2, 0} };

    // Render the game frames and save them as PPM images
    std::cout << "Rendering frames..." << std::endl;
    for (size_t frame=0; frame<360; frame++) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << frame << ".ppm";
        player.a += 2*M_PI/360;

        render(fb, map, player, sprites, tex_walls, tex_monst);
        drop_ppm_image("./out.ppm", fb.img, fb.w, fb.h);
    }

    return 0;
}
*/
