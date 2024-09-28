#include <cassert>

#include "../include/headers/map.h"


static char map[] = "1111111111111111"\
                    "1              1"\
                    "1     1111131111"\
                    "1     1    9   1"\
                    "1     1        1"\
                    "1 9   1        1"\
                    "113111111      1"\
                    "1   1   1      1"\
                    "1   1   1      1"\
                    "1   11311      1"\
                    "1     9 1      1"\
                    "1       1      1"\
                    "111111111      1"\
                    "1              1"\
                    "1              1"\
                    "1111111111111111"; // our game map [1 is a wall, 3 is a door]

Map::Map() : w(16), h(16) {
    assert(sizeof(map) == w*h+1); // +1 for the null terminated string
}

int Map::get(const size_t i, const size_t j) const {
    //assert(i<w && j<h && sizeof(map) == w*h+1);
    return map[i+j*w] - '0';
}

bool Map::is_empty(const size_t i, const size_t j) const {
    assert(i<w && j<h && sizeof(map) == w*h+1);
    return map[i+j*w] == ' ' || map[i+j*w] == '9';
}

/**
 * @brief Checks if the specified coordinates correspond to a door.
 *
 * This function checks the map at the given coordinates (i, j) and its
 * surrounding positions to determine if there is a door present. The door
 * is identified by the value 3 in the map.
 *
 * @param i The x-coordinate to check.
 * @param j The y-coordinate to check.
 * @return An integer indicating the relative position of the door:
 *         - 16 if the door is at (i+16, j+16)
 *         - -16 if the door is at (i-16, j-16)
 *         - 1 if the door is at (i+1, j+1)
 *         - -1 if the door is at (i-1, j-1)
 *        - 0 if no door is found.
 */
std::pair<int, int> Map::check_door(const size_t i, const size_t j) const{
    if (get(i+1, j) == 3) return {1, 0};
    if (get(i-1, j) == 3) return {-1, 0};
    if (get(i, j+1) == 3) return {0, 1};
    if (get(i, j-1) == 3) return {0, -1};
    return {0, 0};
}

void Map::open_door(const size_t i, const size_t j) {
    assert(i<w && j<h && sizeof(map) == w*h+1);
    if (map[i+j*w] == '3') {
        map[i+j*w] = ' ';
    }
}