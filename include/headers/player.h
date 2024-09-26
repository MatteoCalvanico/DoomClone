#ifndef PLAYER_H
#define PLAYER_H

struct Player {
    float x, y; // position
    float a;    // view direction [angle in degrees]
    float fov;  // field of view  [radians]
    int turn, walk; // walk direction and turn direction
};

#endif // PLAYER_H