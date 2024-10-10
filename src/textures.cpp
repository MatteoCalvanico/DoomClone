#include <iostream>
#include <cassert>

#include "../include/sdl/SDL.h"

#include "../include/headers/utils.h"
#include "../include/headers/textures.h"

/**
 * @brief Constructs a Texture object by loading a BMP image file and converting it to the specified format.
 * 
 * This constructor initializes a Texture object by loading an image from the specified file path,
 * converting it to the desired format, and then processing the image data to store it in a vector.
 * The image is expected to be a 32-bit BMP file containing N square textures packed horizontally.
 * 
 * @param filename The path to the BMP image file to be loaded.
 * @param format The desired pixel format for the texture.
 * 
 * The constructor performs the following steps:
 * 1. Loads the BMP image using SDL_LoadBMP.
 * 2. Converts the loaded surface to the specified format using SDL_ConvertSurfaceFormat.
 * 3. Checks if the image is a 32-bit image and if it contains N square textures packed horizontally.
 * 4. Extracts the pixel data from the surface and stores it in a vector.
 * 5. Frees the SDL surfaces used during the process.
 * 
 * If any error occurs during the loading or conversion process, an error message is printed to std::cerr
 * and the constructor returns without initializing the texture data.
 */
Texture::Texture(const std::string filename, const uint32_t format) : img_w(0), img_h(0), count(0), size(0), img() {
    SDL_Surface *tmp = SDL_LoadBMP(filename.c_str());
    if (!tmp) {
        std::cerr << "Error in SDL_LoadBMP: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Surface *surface = SDL_ConvertSurfaceFormat(tmp, format, 0);
    SDL_FreeSurface(tmp);
    if (!surface) {
        std::cerr << "Error in SDL_ConvertSurfaceFormat: " << SDL_GetError() << std::endl;
        return;
    }

    int w = surface->w;
    int h = surface->h;

    if (w*4!=surface->pitch) {
        std::cerr << "Error: the texture must be a 32 bit image" << std::endl;
        SDL_FreeSurface(surface);
        return;
    }
    // Old code that checks if the image contains N square textures packed horizontally (deprecated)
    /*if (w!=h*int(w/h)) {
        std::cerr << "Error: the texture file must contain N square textures packed horizontally" << std::endl;
        SDL_FreeSurface(surface);
        return;
    }*/
    count = w/h;
    size = w/count;
    img_w = w;
    img_h = h;
    uint8_t *pixmap = reinterpret_cast<uint8_t *>(surface->pixels);

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
    SDL_FreeSurface(surface);
}

uint32_t Texture::get(const size_t i, const size_t j, const size_t idx) const {
    assert(i<size && j<size && idx<count);
    return img[i+idx*size+j*img_w];
}

std::vector<uint32_t> Texture::get_scaled_column(const size_t texture_id, const size_t tex_coord, const size_t column_height) const {
    assert(tex_coord<size && texture_id<count);
    std::vector<uint32_t> column(column_height);
    for (size_t y=0; y<column_height; y++) {
        column[y] = get(tex_coord, (y*size)/column_height, texture_id);
    }
    return column;
}