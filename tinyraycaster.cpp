#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>

/**
 * @brief Packs individual color components into a single 32-bit integer.
 *
 * This function takes red, green, blue, and alpha color components as input
 * and combines them into a single 32-bit integer. The alpha component is
 * optional and defaults to 255 (fully opaque) if not provided.
 *
 * @param r The red component of the color (0-255).
 * @param g The green component of the color (0-255).
 * @param b The blue component of the color (0-255).
 * @param a The alpha component of the color (0-255), defaults to 255.
 * @return A 32-bit integer representing the packed color.
 */
uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a=255) {
    return (a<<24) + (b<<16) + (g<<8) + r;
}

/**
 * @brief Unpacks a 32-bit color value into its individual RGBA components.
 * 
 * This function takes a 32-bit unsigned integer representing a color and 
 * extracts the red, green, blue, and alpha components, storing them in 
 * the provided references.
 * 
 * @param color The 32-bit color value to unpack.
 * @param r Reference to a uint8_t where the red component will be stored.
 * @param g Reference to a uint8_t where the green component will be stored.
 * @param b Reference to a uint8_t where the blue component will be stored.
 * @param a Reference to a uint8_t where the alpha component will be stored.
 */
void unpack_color(const uint32_t &color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) {
    r = (color >>  0) & 255;
    g = (color >>  8) & 255;
    b = (color >> 16) & 255;
    a = (color >> 24) & 255;
}

/**
 * @brief Saves an image in PPM format.
 *
 * This function writes the given image data to a file in PPM (Portable Pixmap) format.
 *
 * @param filename The name of the file to save the image to.
 * @param image A vector containing the image data in 32-bit RGBA format.
 * @param w The width of the image.
 * @param h The height of the image.
 *
 * @note The size of the image vector must be equal to w * h.
 */
void drop_ppm_image(const std::string filename, const std::vector<uint32_t> &image, const size_t w, const size_t h) {
    assert(image.size() == w*h);
    std::ofstream ofs;
    ofs.open(filename, std::ofstream::out | std::ofstream::binary);
    ofs << "P6\n" << w << " " << h << "\n255\n";
    for (size_t i = 0; i < h*w; ++i) {
        uint8_t r, g, b, a;
        unpack_color(image[i], r, g, b, a);
        ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
    }
    ofs.close();
}

/**
 * @brief Draws a filled rectangle on an image.
 *
 * This function modifies the provided image by drawing a filled rectangle
 * with the specified dimensions and color at the given coordinates.
 *
 * @param img A reference to a vector of uint32_t representing the image pixels.
 * @param img_w The width of the image.
 * @param img_h The height of the image.
 * @param x The x-coordinate of the top-left corner of the rectangle.
 * @param y The y-coordinate of the top-left corner of the rectangle.
 * @param w The width of the rectangle.
 * @param h The height of the rectangle.
 * @param color The color of the rectangle in uint32_t format.
 *
 * @note The function asserts that the image size matches the provided dimensions
 *       and that the rectangle fits within the image boundaries.
 */
void draw_rectangle(std::vector<uint32_t> &img, const size_t img_w, const size_t img_h, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color) {
    assert(img.size()==img_w*img_h);
    for (size_t i=0; i<w; i++) {
        for (size_t j=0; j<h; j++) {
            size_t cx = x+i;
            size_t cy = y+j;
            assert(cx<img_w && cy<img_h);
            img[cx + cy*img_w] = color;
        }
    }
}

int main() {
    const size_t win_w = 512; // image width
    const size_t win_h = 512; // image height
    std::vector<uint32_t> framebuffer(win_w*win_h, 255); // the image itself

    const size_t map_w = 16; // map width
    const size_t map_h = 16; // map height
    const char map[] = "0000222222220000"\
                       "1              0"\
                       "1     011111   0"\
                       "1     0        0"\
                       "0     0    11000"\
                       "0     3        0"\
                       "0   10000      0"\
                       "0   0   1      0"\
                       "0   0   0      0"\
                       "0   0   1      0"\
                       "0       1      0"\
                       "2       1      0"\
                       "000000000      0"\
                       "0              0"\
                       "0              0"\
                       "0002222222200000"; // our game map
    assert(sizeof(map) == map_w*map_h+1); // +1 for the null terminated string

    // add player position and direction
    // use map_w and map_h to make sure the player is placed in the map boundaries
    float player_x = 2.0;      // player's position
    float player_y = 14.0;     // player's position
    float player_a = 270.0;    // player's view direction [angle in degrees]
    const float fov = M_PI/3;  // field of view [radians, 180°/3 = 60° in this case]


    // Start to render the scene ------------------------------------------------

    for (size_t j = 0; j<win_h; j++) { // make the map white [where the player can go]
        for (size_t i = 0; i<win_w; i++) {
            uint8_t r = 255; 
            uint8_t g = 255; 
            uint8_t b = 255;
            framebuffer[i+j*win_w] = pack_color(r, g, b);
        }
    }

    const size_t rect_w = win_w/map_w;
    const size_t rect_h = win_h/map_h;
    for (size_t j=0; j<map_h; j++) { // draw the map
        for (size_t i=0; i<map_w; i++) {
            if (map[i+j*map_w]==' ') continue; // skip empty spaces
            size_t rect_x = i*rect_w;
            size_t rect_y = j*rect_h;
            draw_rectangle(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, pack_color(0, 0, 0)); // make the map borders black
        }
    }

    // draw the player
    draw_rectangle(framebuffer, win_w, win_h, player_x*rect_w, player_y*rect_h, 5, 5, pack_color(255, 0, 0)); 

    // Heart of 3D engine: draw the player's visibility cone
    /*
    * The player's view direction is represented by a line segment that starts at the player's position 
    * and extends in the direction of the player's view.
    * 
    * The line segment is drawn by iterating over a range of values for t, which represents the distance
    * from the player's position. For each value of t, we calculate the x and y coordinates of the point
    * on the line segment using the parametric equation of a line. We then check if the point is within
    * the map boundaries and stop drawing the line if it hits a wall.
    */
    for (size_t i=0; i<win_w; i++) { 
        // current angle
        float angle = player_a-fov/2 + fov*i/float(win_w); 

        for (float t=0; t<20; t+=.05) { // draw every ray
            float cx = player_x + t*cos(angle);
            float cy = player_y + t*sin(angle);

            if (map[int(cx)+int(cy)*map_w]!=' ') break; // stop if we hit a wall

            size_t pix_x = cx*rect_w;
            size_t pix_y = cy*rect_h;
            framebuffer[pix_x + pix_y*win_w] = pack_color(0, 0, 0); // draw the line segment
        }
    }

    // save the image to a file
    drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

    // End of rendering the scene ----------------------------------------------

    return 0;
}