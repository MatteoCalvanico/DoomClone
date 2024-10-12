#include <cmath>

#include "../include/headers/sprite.h"
#include "../include/headers/player.h"
#include "../include/headers/map.h"

bool Sprite::operator < (const Sprite& s) const {
    return player_dist > s.player_dist;
}

/**
 * @brief Updates the position of the sprite based on the player's position and a given speed.
 * 
 * This function calculates the direction vector from the sprite to the player,
 * normalizes it, and then updates the sprite's position by moving it in the 
 * direction of the player at the specified speed.
 * 
 * @param player A reference to the Player object, which provides the target position.
 * @param speed The speed at which the sprite should move towards the player.
 */
void Sprite::update_position(const Player& player, const Map& map, float speed) {
    float direction_x = player.x - x;
    float direction_y = player.y - y;
    float length = std::sqrt(direction_x * direction_x + direction_y * direction_y);
    
    // Normalize the direction vector
    direction_x /= length;
    direction_y /= length;
    
    // Calculate the new position
    float new_x = x + direction_x * speed;
    float new_y = y + direction_y * speed;
    
    // Check for collisions with walls
    if (map.is_empty(new_x, y)) {
        x = new_x;
    }
    if (map.is_empty(x, new_y)) {
        y = new_y;
    }
}