#ifndef SPRITE_H
#define SPRITE_H

#include <cstdlib>

// Forward declaration of Player class
class Player;

struct Sprite {
    float x, y;
    size_t tex_id;
    float player_dist;
    
    bool operator < (const Sprite& s) const;
    void update_position(const Player& player, float speed);
};

#endif // SPRITE_H