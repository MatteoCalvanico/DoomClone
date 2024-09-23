#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
    std::string output_path = "output/" + filename;
    ofs.open(output_path, std::ofstream::out | std::ofstream::binary);
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
            if (cx>=img_w || cy>=img_h) continue; // no need to check for negative values (unsigned variables)
            img[cx + cy*img_w] = color;
        }
    }
}

/**
 * @brief Loads a texture from a file and stores it in a vector.
 *
 * This function loads a texture from the specified file and stores the pixel data in the provided texture vector.
 * The texture must be a 32-bit image and contain N square textures packed horizontally.
 *
 * @param filename The path to the texture file.
 * @param texture A reference to a vector where the texture data will be stored.
 * @param text_size A reference to a size_t variable where the size of each individual texture will be stored.
 * @param text_cnt A reference to a size_t variable where the count of individual textures will be stored.
 * @return true if the texture was successfully loaded, false otherwise.
 */
bool load_texture(const std::string filename, std::vector<uint32_t> &texture, size_t &text_size, size_t &text_cnt) {
    int nchannels = -1, w, h;
    unsigned char *pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0);
    if (!pixmap) {
        std::cerr << "Error: can not load the textures" << std::endl;
        return false;
    }
    if (4!=nchannels) {
        std::cerr << "Error: the texture must be a 32 bit image" << std::endl;
        stbi_image_free(pixmap);
        return false;
    }
    text_cnt = w/h;
    text_size = w/text_cnt;
    if (w!=h*int(text_cnt)) {
        std::cerr << "Error: the texture file must contain N square textures packed horizontally" << std::endl;
        stbi_image_free(pixmap);
        return false;
    }
    texture = std::vector<uint32_t>(w*h);
    for (int j=0; j<h; j++) {
        for (int i=0; i<w; i++) {
            uint8_t r = pixmap[(i+j*w)*4+0];
            uint8_t g = pixmap[(i+j*w)*4+1];
            uint8_t b = pixmap[(i+j*w)*4+2];
            uint8_t a = pixmap[(i+j*w)*4+3];
            texture[i+j*w] = pack_color(r, g, b, a);
        }
    }
    stbi_image_free(pixmap);
    return true;
}

/**
 * @brief Extracts a vertical column of pixels from a texture atlas.
 *
 * This function extracts a vertical column of pixels from a specified texture
 * in a texture atlas. The texture atlas is represented by a single image
 * containing multiple textures of the same size arranged horizontally.
 *
 * @param img The texture atlas image represented as a vector of pixels.
 * @param texsize The size (width and height) of each individual texture in the atlas.
 * @param ntextures The number of textures in the atlas.
 * @param texid The ID of the texture to extract the column from (0-based index).
 * @param texcoord The x-coordinate within the texture to extract the column from.
 * @param column_height The height of the column to extract.
 * @return A vector containing the extracted column of pixels.
 *
 * @note The function assumes that the texture atlas is a square image with
 *       dimensions texsize * ntextures by texsize.
 * @note The function asserts that the size of the img vector matches the expected
 *       dimensions of the texture atlas and that texcoord and texid are within
 *       valid ranges.
 */
std::vector<uint32_t> texture_column(const std::vector<uint32_t> &img, const size_t texsize, const size_t ntextures, const size_t texid, const size_t texcoord, const size_t column_height) {
    const size_t img_w = texsize*ntextures;
    const size_t img_h = texsize;
    assert(img.size()==img_w*img_h && texcoord<texsize && texid<ntextures);
    std::vector<uint32_t> column(column_height);
    for (size_t y=0; y<column_height; y++) {
        size_t pix_x = texid*texsize + texcoord;
        size_t pix_y = (y*texsize)/column_height;
        column[y] = img[pix_x + pix_y*img_w];
    }
    return column;
}


int main() {
    const size_t win_w = 1024; // image width
    const size_t win_h = 512;  // image height
    std::vector<uint32_t> framebuffer(win_w*win_h, pack_color(255, 255, 255)); // the image itself [white background]

    const size_t map_w = 16; // map width
    const size_t map_h = 16; // map height
    const char map[] = "1111111111111111"\
                       "1              1"\
                       "1     1111131111"\
                       "1     1        1"\
                       "1     1        1"\
                       "1     1        1"\
                       "113111111      1"\
                       "1   1   1      1"\
                       "1   1   1      1"\
                       "1   11311      1"\
                       "1       1      1"\
                       "1       1      1"\
                       "111111111      1"\
                       "1              1"\
                       "1              1"\
                       "1111111111111111"; // our game map [1 is a wall, 3 is a door]
    assert(sizeof(map) == map_w*map_h+1); // +1 for the null terminated string

    // add player position and direction
    // use map_w and map_h to make sure the player is placed in the map boundaries
    float player_x = 2.0;      // player's position
    float player_y = 14.0;     // player's position
    float player_a = 270.0;    // player's view direction [angle in degrees]
    const float fov = M_PI/3;  // field of view [radians, 180°/3 = 60° in this case]

    std::vector<uint32_t> walltext; // textures for the walls
    size_t walltext_size;           // texture dimensions (it is a square)
    size_t walltext_cnt;            // number of different textures in the image
    if (!load_texture("./texture/walltext.png", walltext, walltext_size, walltext_cnt)) {
        std::cerr << "Failed to load wall textures" << std::endl;
        return -1;
    }


    // Start to render the scene ------------------------------------------------

    const size_t rect_w = win_w/(map_w*2);
    const size_t rect_h = win_h/map_h;

    for (size_t j=0; j<map_h; j++) { // draw the map
        for (size_t i=0; i<map_w; i++) {
            if (map[i+j*map_w]==' ') continue; // skip empty spaces
            size_t rect_x = i*rect_w;
            size_t rect_y = j*rect_h;

            size_t texid = map[i+j*map_w] - '0';
            assert(texid<walltext_cnt);
            // Fill the wall with the color from the upper left pixel of the texture #texid
            draw_rectangle(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, walltext[texid*walltext_size]);
        }
    }


    // draw the player
    draw_rectangle(framebuffer, win_w, win_h, player_x*rect_w, player_y*rect_h, 5, 5, pack_color(255, 0, 0)); 

    // Heart of 3D engine: draw the player's visibility cone AND "3D" view
    /*
    * The player's view direction is represented by a line segment that starts at the player's position 
    * and extends in the direction of the player's view.
    * 
    * The line segment is drawn by iterating over a range of values for t, which represents the distance
    * from the player's position. For each value of t, we calculate the x and y coordinates of the point
    * on the line segment using the parametric equation of a line. We then check if the point is within
    * the map boundaries and stop drawing the line if it hits a wall.
    * 
    * The angle of the line segment is calculated based on the player's view direction and the field of view.
    * We iterate over a range of values for i to draw multiple rays that cover the player's field of view.
    * 
    * 
    * To give a sense of depth, we draw vertical columns for each ray that hits a wall. The height of the
    * column is inversely proportional to the distance of the wall, creating the illusion of 3D.
    */
    for (size_t i=0; i<win_w/2; i++) { 
        // current angle
        float angle = player_a-fov/2 + fov*i/float(win_w/2); 

        for (float t=0; t<20; t+=.05) { // draw every ray
            float cx = player_x + t*cos(angle);
            float cy = player_y + t*sin(angle);

            int pix_x = cx*rect_w;
            int pix_y = cy*rect_h;
            framebuffer[pix_x + pix_y*win_w] = pack_color(0, 0, 0); // draw the line segment

            if (map[int(cx)+int(cy)*map_w]!=' ') { // our ray touches a wall, so draw the vertical column to create an illusion of 3D
                size_t texid = map[int(cx)+int(cy)*map_w] - '0';
                assert(texid<walltext_cnt);

                size_t column_height = win_h/(t*cos(angle-player_a)); // for fish-eye effect correction

                float hitx = cx - floor(cx+.5); // hitx and hity contain (signed) fractional parts of cx and cy,
                float hity = cy - floor(cy+.5); // they vary between -0.5 and +0.5, and one of them is supposed to be very close to 0
                int x_texcoord = hitx*walltext_size;
                if (std::abs(hity)>std::abs(hitx)) { // we need to determine whether we hit a "vertical" or a "horizontal" wall (w.r.t the map)
                    x_texcoord = hity*walltext_size;
                }
                if (x_texcoord<0) x_texcoord += walltext_size; // do not forget x_texcoord can be negative
                assert(x_texcoord>=0 && x_texcoord<(int)walltext_size);
                std::vector<uint32_t> column = texture_column(walltext, walltext_size, walltext_cnt, texid, x_texcoord, column_height);
                pix_x = win_w/2+i;
                for (size_t j=0; j<column_height; j++) {
                    pix_y = j + win_h/2-column_height/2;
                    if (pix_y<0 || pix_y>=(int)win_h) continue;
                    framebuffer[pix_x + pix_y*win_w] = column[j];
                }
                break;
            }
        }
    }

    drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

    // End of rendering the scene ----------------------------------------------

    return 0;
}