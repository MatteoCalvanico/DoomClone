# DoomClone
A simple C++ game similar to Doom and Wolfenstein 3D.

## Commands AND rule
TODO

### Info
This game use **SDL2** to work.
Before build the project you need to follow this [video](https://www.youtube.com/watch?v=9Ca-RVPwnBE&ab_channel=vader) to setup the header and lib file to make the game work; after that you can use the Makefile or this command:
```sh
g++ -Iinclude -Iinclude/sdl -Iinclude/headers -Llib -o DoomClone src/*.cpp -lmingw32 -lSDL2 -lSDL2_image -mconsole
```
directly in the project's root to compile.

#### Credits
This game was created thanks to this tutorial: [ssloy/tinycaster](https://github.com/ssloy/tinyraycaster), which allowed me to learn the basics of game programming and 3D engines.