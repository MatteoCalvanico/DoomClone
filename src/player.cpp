#include <cmath>
#include <SDL.h>

#include "../include/headers/player.h"

Player::Player(float x, float y, float a, float fov) : x(x), y(y), a(a), fov(fov), turn(0), walk(0) {}

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

    // Reset shooting after 1 second
    if (shooting && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - shooting_time).count() >= 100) {
        shooting = false;
    }
}

/**
 * @brief Handles player input events.
 * 
 * This function processes various SDL events to control the player's actions
 * such as movement, turning, shooting, and interacting with the map.
 * 
 * @param event The SDL_Event to handle.
 * @param map The game map, used for interactions like opening doors.
 * 
 * Event Handling:
 * - SDL_KEYUP: Stops movement or turning when 'a', 'd', 'w', or 's' keys are released.
 * - SDL_KEYDOWN: 
 *   - 'a': Turns the player left.
 *   - 'd': Turns the player right.
 *   - 'w': Moves the player forward.
 *   - 's': Moves the player backward.
 *   - 'f': Interacts with the map, such as opening doors if the player is near one.
 * - SDL_MOUSEBUTTONDOWN: 
 *   - SDL_BUTTON_LEFT or SDL_BUTTON_RIGHT: Initiates shooting action.
 */
void Player::handle_event(const SDL_Event &event, Map &map) {
    if (SDL_KEYUP == event.type) {
        if ('a' == event.key.keysym.sym || 'd' == event.key.keysym.sym) turn = 0;
        if ('w' == event.key.keysym.sym || 's' == event.key.keysym.sym) walk = 0;
    }
    if (SDL_KEYDOWN == event.type) {
        if ('a' == event.key.keysym.sym) turn = -1;
        if ('d' == event.key.keysym.sym) turn = 1;
        if ('w' == event.key.keysym.sym) walk = 1;
        if ('s' == event.key.keysym.sym) walk = -1;
        if ('f' == event.key.keysym.sym) {
            size_t i = static_cast<size_t>(x);
            size_t j = static_cast<size_t>(y);
            if (map.get(i, j) == 9) { // Open the door if the player is standing in front of it
                auto [di, dj] = map.check_door(i, j);
                if (di != 0 || dj != 0) {
                    map.open_door(i + di, j + dj);
                }
            }
        }
    }
    if (SDL_MOUSEBUTTONDOWN == event.type) {
        if (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT) {
            shooting = true;
            shooting_time = std::chrono::high_resolution_clock::now();
        }
    }
}