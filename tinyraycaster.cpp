#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <sstream>
#include <iomanip>

#include "map.h"
#include "utils.h"
#include "player.h"
#include "framebuffer.h"
#include "textures.h"

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
int wall_x_texcoord(const float x, const float y, Texture &tex_walls) {
    float hitx = x - floor(x+.5); // hitx and hity contain (signed) fractional parts of x and y,
    float hity = y - floor(y+.5); // they vary between -0.5 and +0.5, and one of them is supposed to be very close to 0
    int tex = hitx*tex_walls.size;

    if (std::abs(hity)>std::abs(hitx)) // we need to determine whether we hit a "vertical" or a "horizontal" wall (w.r.t the map)
        tex = hity*tex_walls.size;
    
    if (tex<0) // do not forget x_texcoord can be negative, fix that
        tex += tex_walls.size;

    assert(tex>=0 && tex<(int)tex_walls.size);

    return tex;
}

/**
 * @brief Renders the game frame including the map and the player's visibility cone.
 * 
 * This function clears the framebuffer, draws the map, and then renders the player's 
 * visibility cone and the 3D view of the scene.
 * 
 * @param fb The framebuffer to render to.
 * @param map The map of the game world.
 * @param player The player object containing position and viewing angle.
 * @param tex_walls The texture containing wall textures.
 * 
 * The rendering process involves:
 * - Clearing the framebuffer to a white color.
 * - Drawing the map cells on the left half of the screen.
 * - Ray marching to determine visible walls and drawing the 3D view on the right half of the screen.
 * - Drawing the visibility cone on the map.
 */
void render(FrameBuffer &fb, Map &map, Player &player, Texture &tex_walls) {
    fb.clear(pack_color(255, 255, 255)); // clear the screen

    const size_t rect_w = fb.w/(map.w*2); // size of one map cell on the screen
    const size_t rect_h = fb.h/map.h;

    for (size_t j=0; j<map.h; j++) {  // draw the map
        for (size_t i=0; i<map.w; i++) {

            if (map.is_empty(i, j)) continue; // skip empty spaces

            size_t rect_x = i*rect_w;
            size_t rect_y = j*rect_h;
            size_t texid = map.get(i, j);

            assert(texid<tex_walls.count);

            // Fill the wall with the color from the upper left pixel of the texture #texid
            fb.draw_rectangle(rect_x, rect_y, rect_w, rect_h, tex_walls.get(0, 0, texid));
        }
    }

    // Draw the visibility cone and the 3D view
    for (size_t i=0; i<fb.w/2; i++) { 

        float angle = player.a-player.fov/2 + player.fov*i/float(fb.w/2); // current angle

        // Ray casting - find the distance to the first wall in the specific direction
        for (float t=0; t<20; t+=.01) {
            float x = player.x + t*cos(angle);
            float y = player.y + t*sin(angle);
            fb.set_pixel(x*rect_w, y*rect_h, pack_color(160, 160, 160)); // this draws the visibility cone

            if (map.is_empty(x, y)) continue;

            size_t texid = map.get(x, y); // our ray touches a wall, so draw the vertical column to create an illusion of 3D
            assert(texid<tex_walls.count);

            size_t column_height = fb.h/(t*cos(angle-player.a)); // for fish-eye effect correction

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
}

int main() {

    // Create a framebuffer, player, map, and wall textures
    FrameBuffer fb{1024, 512, std::vector<uint32_t>(1024*512, pack_color(255, 255, 255))};
    Player player{2, 14, 270, M_PI/3.};
    Map map;
    Texture tex_walls("texture/walltext.png");

    if (!tex_walls.count) {
        std::cerr << "Failed to load wall textures" << std::endl;
        return -1;
    }

    // Render the game frames and save them as PPM images
    for (size_t frame=0; frame<360; frame++) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << frame << ".ppm";
        player.a += 2*M_PI/360;

        render(fb, map, player, tex_walls);
        drop_ppm_image(ss.str(), fb.img, fb.w, fb.h);
    }

    return 0;
}

