#include <cassert>

#include "../include/headers/framebuffer.h"
#include "../include/headers/utils.h"

/**
 * @brief Sets the color of a specific pixel in the framebuffer.
 * 
 * This function sets the color of the pixel located at the specified (x, y) 
 * coordinates in the framebuffer. The framebuffer is represented as a 1D 
 * array where the pixel at (x, y) is mapped to the index x + y * w.
 * 
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param color The color to set the pixel to, represented as a 32-bit unsigned integer.
 * 
 * @pre The framebuffer image size must be equal to the product of its width and height.
 * @pre The x-coordinate must be less than the width of the framebuffer.
 * @pre The y-coordinate must be less than the height of the framebuffer.
 */
void FrameBuffer::set_pixel(const size_t x, const size_t y, const uint32_t color) {
    assert(img.size()==w*h && x<w && y<h);
    img[x+y*w] = color;
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
void FrameBuffer::draw_rectangle(const size_t rect_x, const size_t rect_y, const size_t rect_w, const size_t rect_h, const uint32_t color) {
    assert(img.size()==w*h);
    for (size_t i=0; i<rect_w; i++) {
        for (size_t j=0; j<rect_h; j++) {
            size_t cx = rect_x+i;
            size_t cy = rect_y+j;
            if (cx<w && cy<h) // no need to check for negative values (unsigned variables)
                set_pixel(cx, cy, color);
        }
    }
}

/**
 * @brief Clears the framebuffer with a specified color.
 * 
 * This function fills the entire framebuffer with the specified color.
 * 
 * @param color The color to fill the framebuffer with, represented as a 32-bit unsigned integer.
 */
void FrameBuffer::clear(const uint32_t color) {
    img = std::vector<uint32_t>(w*h, color);
}