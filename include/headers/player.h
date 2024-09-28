#ifndef PLAYER_H
#define PLAYER_H

#include <SDL.h>

#include "map.h"

class Player {
public:
    float x, y;     // position
    float a;        // view direction [angle in degrees]
    float fov;      // field of view  [radians]
    int turn, walk; // walk direction and turn direction

    Player(float x, float y, float a, float fov);

    void update_position(const Map &map);
    void handle_event(const SDL_Event &event);
};

#endif // PLAYER_H