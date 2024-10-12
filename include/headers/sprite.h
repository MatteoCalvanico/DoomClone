#ifndef SPRITE_H
#define SPRITE_H

#include <cstdlib>

// Forward declaration of Player class
class Player;
class Map;

struct Sprite {
    float x, y;
    size_t tex_id;
    float player_dist;
    
    bool operator < (const Sprite& s) const;
    void update_position(const Player& player, const Map& map, float speed);
};

#endif // SPRITE_H