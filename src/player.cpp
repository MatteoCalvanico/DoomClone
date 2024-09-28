#include <cmath>
#include <SDL.h>

#include "../include/headers/player.h"

Player::Player(float x, float y, float a, float fov)
    : x(x), y(y), a(a), fov(fov), turn(0), walk(0) {}

/**
 * @brief Updates the player's position based on the current movement and direction.
 *
 * This function updates the player's position on the map by adjusting the player's
 * coordinates according to the current walking and turning values. The player's
 * angle is modified by the turning value, and the new position is calculated using
 * trigonometric functions. The function ensures that the new position is within the
 * bounds of the map and that the target positions are empty before updating the
 * player's coordinates.
 *
 * @param map A reference to the Map object representing the game world.
 */
void Player::update_position(const Map &map) {
    a += float(turn) * .1; // TODO measure elapsed time and modify the speed accordingly
    float nx = x + walk * cos(a) * .1;
    float ny = y + walk * sin(a) * .1;

    if (int(nx) >= 0 && int(nx) < int(map.w) && int(ny) >= 0 && int(ny) < int(map.h)) {
        if (map.is_empty(nx, y)) x = nx;
        if (map.is_empty(x, ny)) y = ny;
    }
}

/**
 * @brief Handles keyboard events for player movement.
 *
 * This function processes SDL keyboard events to control the player's movement.
 * It updates the player's turning and walking states based on the key pressed or released.
 *
 * @param event The SDL_Event object containing the event data.
 *
 * The function handles the following keys:
 * - 'a': Turn left
 * - 'd': Turn right
 * - 'w': Walk forward
 * - 's': Walk backward
 *
 * When a key is released (SDL_KEYUP), the corresponding movement state is reset to 0.
 * When a key is pressed (SDL_KEYDOWN), the corresponding movement state is set to:
 * - -1 for turning left or walking backward
 * - 1 for turning right or walking forward
 */
void Player::handle_event(const SDL_Event &event) {
    if (SDL_KEYUP == event.type) {
        if ('a' == event.key.keysym.sym || 'd' == event.key.keysym.sym) turn = 0;
        if ('w' == event.key.keysym.sym || 's' == event.key.keysym.sym) walk = 0;
    }
    if (SDL_KEYDOWN == event.type) {
        if ('a' == event.key.keysym.sym) turn = -1;
        if ('d' == event.key.keysym.sym) turn = 1;
        if ('w' == event.key.keysym.sym) walk = 1;
        if ('s' == event.key.keysym.sym) walk = -1;
    }
}