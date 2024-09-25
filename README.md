# DoomClone
A simple C++ game similar to Doom and Wolfenstein 3D

## Info
This game use **SDL2** to work, to make easier the build, *the library is already in the repository*.
You can use the Makefile or this command:
```sh
g++ -Iinclude -Iinclude/sdl -Iinclude/headers -Llib -o DoomClone src/*.cpp -lmingw32 -lSDL2 -lSDL2_image -mconsole
```
directly in the project's root to compile