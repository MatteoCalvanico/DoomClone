#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>
#include "utils.h"

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
uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) {
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