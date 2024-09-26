#include <cassert>

#include "../include/headers/map.h"


static const char map[] = "1111111111111111"\
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

Map::Map() : w(16), h(16) {
    assert(sizeof(map) == w*h+1); // +1 for the null terminated string
}

int Map::get(const size_t i, const size_t j) const {
    assert(i<w && j<h && sizeof(map) == w*h+1);
    return map[i+j*w] - '0';
}

bool Map::is_empty(const size_t i, const size_t j) const {
    assert(i<w && j<h && sizeof(map) == w*h+1);
    return map[i+j*w] == ' ';
}