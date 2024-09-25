#ifndef PLAYER_H
#define PLAYER_H

struct Player {
    float x, y; // position
    float a;    // view direction [angle in degrees]
    float fov;  // field of view  [radians]
};

#endif // PLAYER_H