#ifndef MAP_H
#define MAP_H

#include <cstdlib>
#include <utility>

struct Map {
    size_t w, h; // overall map dimensions

    Map();

    int get(const size_t i, const size_t j) const;
    
    bool is_empty(const size_t i, const size_t j) const;

    std::pair<int, int> check_door(const size_t i, const size_t j) const;

    void open_door(const size_t i, const size_t j);
};

#endif // MAP_H