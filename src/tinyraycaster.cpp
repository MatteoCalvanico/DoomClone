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

/**<
 * @brief Draws a gun sprite onto the framebuffer.
 *
 * This function scales and draws a gun sprite from the provided texture onto the framebuffer.
 * The gun sprite is centered horizontally and positioned at the bottom of the screen.
 * The sprite is scaled by a factor specified by `scale_factor`.
 * White pixels in the sprite are considered transparent and are not drawn.
 *
 * @param fb The framebuffer to draw the gun sprite onto.
 * @param tex_gun The texture containing the gun sprite.
 * @param use_firing_sprite A flag indicating whether to use the firing sprite.
 */
void draw_gun(FrameBuffer &fb, const Texture &tex_gun, bool use_firing_sprite) {
    float scale_factor = 1; // Adjust this value to make the weapon larger

    // Determine the sprite index based on the use_firing_sprite flag
    size_t sprite_index = use_firing_sprite ? 1 : 0;

    // Calculate the new dimensions of the gun sprite
    size_t gun_w = static_cast<size_t>(tex_gun.img_w / 2 * scale_factor);
    size_t gun_h = static_cast<size_t>(tex_gun.img_h * scale_factor);

    // Calculate the centered position of the gun sprite
    size_t gun_x = (fb.w - gun_w) / 2;
    size_t gun_y = fb.h - gun_h;

    // Draw the scaled gun sprite
    for (size_t y = 0; y < gun_h; y++) {
        for (size_t x = 0; x < gun_w; x++) {
            // Calculate the corresponding pixel in the original sprite
            size_t orig_x = static_cast<size_t>(x / scale_factor);
            size_t orig_y = static_cast<size_t>(y / scale_factor);

            // Get the pixel from the correct sprite based on sprite_index
            uint32_t color = tex_gun.get(orig_x, orig_y, sprite_index);

            // Skip the white pixels
            if (color != pack_color(255, 255, 255)) {
                fb.set_pixel(gun_x + x, gun_y + y, color);
            }
        }
    }
}

/**
 * @brief Renders the game frame.
 * 
 * This function is responsible for rendering the entire game frame, including the floor, ceiling, walls, sprites, and HUD elements.
 * 
 * @param fb The framebuffer to render to.
 * @param gs The current game state, containing player information, textures, and map data.
 * @param renderer The SDL renderer used for rendering.
 * 
 * The rendering process includes:
 * - Clearing the screen.
 * - Drawing the floor and ceiling using ray casting.
 * - Drawing the walls using Digital Differential Analysis (DDA) for ray casting.
 * - Drawing the sprites (monsters) in the game.
 * - Drawing the map overlay on top of the 3D view.
 * - Drawing the player's gun on the screen.
 * - Checking if the player is near a door and showing a prompt to open it.
 */
void render(FrameBuffer &fb, const GameState &gs, SDL_Renderer* renderer) {
    fb.clear(pack_color(255, 255, 255)); // clear the screen

    const Texture &tex_gun = gs.tex_gun;

    // size of one map cell on the screen
    const size_t cell_w = fb.w / (gs.map.w * 4);
    const size_t cell_h = fb.h / (gs.map.h * 4);

    std::vector<float> depth_buffer(fb.w, 1e3); // buffer to store the Z-coordinate based on the ray casting

    // player's position
    float posX = gs.player.x;       
    float posY = gs.player.y;       

    float playerViewDir = gs.player.a; // player's view direction

    float playerFov = gs.player.fov;   // player's field of view

    const float aspect_ratio = float(fb.h) / fb.w; // aspect ratio of the screen

    // direction vector
    float dirX = cos(playerViewDir);
    float dirY = sin(playerViewDir); 

    // camera plane
    float planeX = cos(playerViewDir + M_PI / 2) * playerFov;
    float planeY = sin(playerViewDir + M_PI / 2) * playerFov;


    // -------------- 3D engine --------------
    // Draw the floor and ceiling
    for (int y = 0; y < fb.h; y++) {
        float rayDirX0 = dirX - planeX; // vectors for right and left sides of the camera plane
        float rayDirY0 = dirY - planeY; // the 2d ray dir without the distortion correction
        float rayDirX1 = dirX + planeX; // vectors for right and left sides of the camera plane
        float rayDirY1 = dirY + planeY; // the 2d ray dir without the distortion correction
        
        int p = y - fb.h / 2;
        float posZ = 0.5 * fb.h;
        float rowDistance = posZ / p;

        float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / fb.w;
        float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / fb.w;

        float floorX = posX + rowDistance * rayDirX0;
        float floorY = posY + rowDistance * rayDirY0;

        // Draw the floor from the bottom to the center of the screen
        for (int x = 0; x < fb.w; ++x) {
            int cellX = (int)(floorX);
            int cellY = (int)(floorY);

            int tx = (int)(gs.tex_walls.size * (floorX - cellX)) & (gs.tex_walls.size - 1); 
            int ty = (int)(gs.tex_walls.size * (floorY - cellY)) & (gs.tex_walls.size - 1); 

            floorX += floorStepX;
            floorY += floorStepY;

            // textures for the floor and ceiling
            int floorTexture = 5;
            int ceilingTexture = 2;
            uint32_t color;

            // floor
            color = gs.tex_walls.get(tx, ty, floorTexture);
            color = (color >> 1) & 8355711; // make a bit darker
            fb.set_pixel(x, y, color);

            // ceiling (symmetrical, at screenHeight - y - 1 instead of y)
            color = gs.tex_walls.get(tx, ty, ceilingTexture);
            color = (color >> 1) & 8355711; // make a bit darker
            fb.set_pixel(x, fb.h - y - 1, color);
        }
    }

    // Draw the walls - Ray casting with DDA
    for (size_t x = 0; x < fb.w; x++) {
        float ray_angle = (playerViewDir - playerFov / 2) + (x / float(fb.w)) * playerFov; // current ray angle

        // calculate the direction of the ray
        float ray_dir_x = cos(ray_angle);
        float ray_dir_y = sin(ray_angle);

        // the cell of the map in which we are
        int map_x = int(posX); 
        int map_y = int(posY); 

        float side_dist_x; // length of ray from current position to next x or y-side
        float side_dist_y; // length of ray from current position to next x or y-side

        float delta_dist_x = std::abs(1 / ray_dir_x); // length of ray from one x or y-side to next x or y-side 
        float delta_dist_y = std::abs(1 / ray_dir_y); // length of ray from one x or y-side to next x or y-side

        float perp_wall_dist; // length of the ray from the player to the wall

        // direction to increment x and y (either +1 or -1)
        int step_x; 
        int step_y;

        bool hit = false; // was there a wall hit?
        int side;         // was a NS or a EW wall hit?

        // calculate step and initial sideDist [X]
        if (ray_dir_x < 0) {
            step_x = -1;
            side_dist_x = (posX - map_x) * delta_dist_x;
        } else {
            step_x = 1;
            side_dist_x = (map_x + 1.0 - posX) * delta_dist_x;
        }

        // calculate step and initial sideDist [Y]
        if (ray_dir_y < 0) {
            step_y = -1;
            side_dist_y = (posY - map_y) * delta_dist_y;
        } else {
            step_y = 1;
            side_dist_y = (map_y + 1.0 - posY) * delta_dist_y;
        }

        // perform Digital Differential Analysis (DDA)
        while (!hit) {
            if (side_dist_x < side_dist_y) { 
                side_dist_x += delta_dist_x;
                map_x += step_x;
                side = 0;
            } else {
                side_dist_y += delta_dist_y;
                map_y += step_y;
                side = 1;
            }

            // check if the ray has hit a wall
            int map_value = gs.map.get(map_x, map_y);        
            if (map_value > 0 && map_value != 9) hit = true; // 9 is where the player stay to open the door
        }

        // calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
        if (side == 0) 
            perp_wall_dist = (map_x - posX + (1 - step_x) / 2) / ray_dir_x;
        else
            perp_wall_dist = (map_y - posY + (1 - step_y) / 2) / ray_dir_y;
        
        depth_buffer[x] = perp_wall_dist; // save the distance for the current column

        int line_height = (int)(fb.h / perp_wall_dist); // height of the line to draw on the screen

        int draw_start = -line_height / 2 + fb.h / 2;
        if (draw_start < 0) draw_start = 0;     
        int draw_end = line_height / 2 + fb.h / 2;
        if (draw_end >= fb.h) draw_end = fb.h - 1;

        // calculate value of wall_x
        int tex_x = wall_x_texcoord(posX + ray_dir_x * perp_wall_dist, posY + ray_dir_y * perp_wall_dist, gs.tex_walls);

        // draw the wall slice
        for (int y = draw_start; y < draw_end; y++) {
            int d = y * 256 - fb.h * 128 + line_height * 128;
            int tex_y = ((d * gs.tex_walls.size) / line_height) / 256;
            uint32_t color = gs.tex_walls.get(tex_x, tex_y, gs.map.get(map_x, map_y));
            fb.set_pixel(x, y, color);
        }
    }
    // --------------------------------------

    // Draw the sprites
    for (const auto &sprite : gs.monsters) {
        draw_sprite(sprite, gs.player, fb, depth_buffer, gs.tex_monst);
    }

    // Draw the map on top of the 3D view
    draw_map(fb, gs.monsters, gs.tex_walls, gs.map, gs.player, cell_w, cell_h);

    // Show gun on the screen
    draw_gun(fb, tex_gun, gs.player.shooting);

    // Check if the player is near a door and show "F to open" - TODO: Fix this
    size_t i = static_cast<size_t>(posX);
    size_t j = static_cast<size_t>(posY);
}