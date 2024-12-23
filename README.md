# DoomClone
A simple C++ game similar to Doom and Wolfenstein 3D.
![Screenshot](output/12-10-24.png)

## Commands AND rule
- **WASD**: Move
- **SX or DX mouse**: Fire
- **F**: Open the doors
- **ESC**: Quit

## Info AND Compilation
This game use **SDL2, SDL2_Image AND SDL2_ttf** to work.
Before build the project you need to follow this [video](https://www.youtube.com/watch?v=9Ca-RVPwnBE&ab_channel=vader) to setup the header and lib file to make the game work; after that you can use the Makefile or this command:
```sh
g++ -Iinclude -Iinclude/sdl -Iinclude/headers -Llib -o DoomClone src/*.cpp -lmingw32 -lSDL2 -lSDL2_image -lSDL2_ttf -mconsole
```
directly in the project's root to compile.

*!!! You need to do the same step show in the video but with SDL2_ttf !!!*

**[NEED FIX]**
**Or** you can use Docker for *Build and Run*:
```sh
docker build -t doomclone .
docker run -it --rm doomclone
```

### Bug to fix
- The player can shoot and kill enemies even if they are behind a wall
- Enemies get stuck in the wall

#### Credits
This game was created thanks to this tutorial: [ssloy/tinycaster](https://github.com/ssloy/tinyraycaster), which allowed me to learn the basics of game programming and 3D engines.


To better understand the principles of *raycaster* I recommend this page: [Lode's Computer Graphics Tutorial](https://lodev.org/cgtutor/raycasting.html) 
