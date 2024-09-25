#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>

#define STB_IMAGE_IMPLEMENTATION

#include "../include/headers/stb_image.h"
#include "../include/headers/utils.h"
#include "../include/headers/textures.h"

/**
 * @brief Constructs a Texture object by loading an image from a file.
 *
 * This constructor initializes a Texture object by loading an image from the specified file.
 * It performs several checks to ensure the image is valid and meets the required criteria:
 * - The image must be successfully loaded.
 * - The image must be a 32-bit image (4 channels).
 * - The image width must be an integer multiple of its height.
 *
 * If any of these checks fail, an error message is printed to std::cerr, and the constructor returns early.
 * Otherwise, the image data is processed and stored in the `img` member variable.
 *
 * @param filename The path to the image file to load.
 */
Texture::Texture(const std::string filename) : img_w(0), img_h(0), count(0), size(0), img() {
    int nchannels = -1, w, h;
    unsigned char *pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0);

    if (!pixmap) {
        std::cerr << "Error: can not load the textures" << std::endl;
        return;
    }

    if (4!=nchannels) {
        std::cerr << "Error: the texture must be a 32 bit image" << std::endl;
        stbi_image_free(pixmap);
        return;
    }

    if (w!=h*int(w/h)) {
        std::cerr << "Error: the texture file must contain N square textures packed horizontally" << std::endl;
        stbi_image_free(pixmap);
        return;
    }

    count = w/h;
    size = w/count;
    img_w = w;
    img_h = h;
    img = std::vector<uint32_t>(w*h);

    for (int j=0; j<h; j++) {
        for (int i=0; i<w; i++) {
            uint8_t r = pixmap[(i+j*w)*4+0];
            uint8_t g = pixmap[(i+j*w)*4+1];
            uint8_t b = pixmap[(i+j*w)*4+2];
            uint8_t a = pixmap[(i+j*w)*4+3];
            img[i+j*w] = pack_color(r, g, b, a);
        }
    }
    stbi_image_free(pixmap);
}

uint32_t &Texture::get(const size_t i, const size_t j, const size_t idx) {
    assert(i<size && j<size && idx<count);
    return img[i+idx*size+j*img_w];
}

std::vector<uint32_t> Texture::get_scaled_column(const size_t texture_id, const size_t tex_coord, const size_t column_height) {
    assert(tex_coord<size && texture_id<count);
    std::vector<uint32_t> column(column_height);
    for (size_t y=0; y<column_height; y++) {
        column[y] = get(tex_coord, (y*size)/column_height, texture_id);
    }
    return column;
}