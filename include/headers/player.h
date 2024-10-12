#ifndef PLAYER_H
#define PLAYER_H

#include <SDL.h>
#include <chrono>
#include <vector>
#include <sprite.h>

// Forward declaration of Sprite class
class Sprite;

#include "map.h"

class Player {
public:
    float x, y;     // position
    float a;        // view direction [angle in degrees]
    float fov;      // field of view  [radians]
    int turn, walk; // walk direction and turn direction
    bool shooting;  // shooting state
    std::chrono::time_point<std::chrono::high_resolution_clock> shooting_time; // time when shooting started

    Player(float x, float y, float a, float fov);

    void update_position(const Map &map);
    void handle_event(const SDL_Event &event, Map &map, std::vector<Sprite> &monsters);
    void check_and_remove_hit_monster(std::vector<Sprite> &monsters);
};

#endif // PLAYER_H