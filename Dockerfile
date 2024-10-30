FROM ubuntu:latest

# Set the working directory
WORKDIR /usr/src/app

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    wget \
    unzip

# Copy the current directory contents into the container at /usr/src/app
COPY . .

# Ensure the directories exist
RUN mkdir -p /usr/src/app/include/sdl /usr/src/app/lib

# Download and extract SDL
RUN wget https://github.com/libsdl-org/SDL/releases/download/release-2.30.8/SDL2-devel-2.30.8-VC.zip -O SDL2.zip && \
    unzip SDL2.zip -d SDL2 && \
    cp -r SDL2/SDL2-2.30.8/include/* /usr/src/app/include/sdl && \
    cp -r SDL2/SDL2-2.30.8/lib/x64/* /usr/src/app/lib && \
    cp SDL2/SDL2-2.30.8/lib/x64/*.dll /usr/src/app/

# Download and extract SDL_Image
RUN wget https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-devel-2.8.2-VC.zip -O SDL2_image.zip && \
    unzip SDL2_image.zip -d SDL2_image && \
    cp -r SDL2_image/SDL2_image-2.8.2/include/* /usr/src/app/include/sdl && \
    cp -r SDL2_image/SDL2_image-2.8.2/lib/x64/* /usr/src/app/lib && \
    cp SDL2_image/SDL2_image-2.8.2/lib/x64/*.dll /usr/src/app/

# Download and extract SDL_ttf
RUN wget https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-devel-2.22.0-VC.zip -O SDL2_ttf.zip && \
    unzip SDL2_ttf.zip -d SDL2_ttf && \
    cp -r SDL2_ttf/SDL2_ttf-2.22.0/include/* /usr/src/app/include/sdl && \
    cp -r SDL2_ttf/SDL2_ttf-2.22.0/lib/x64/* /usr/src/app/lib && \
    cp SDL2_ttf/SDL2_ttf-2.22.0/lib/x64/*.dll /usr/src/app/

# Set the library path environment variable
ENV LD_LIBRARY_PATH=/usr/src/app/lib

# Update the shared library cache
RUN ldconfig

# Compile the game
RUN g++ -Iinclude -Iinclude/sdl -Iinclude/headers -L/usr/src/app/lib -o DoomClone src/*.cpp -lSDL2 -lSDL2_image -lSDL2_ttf

# Make port 80 available to the world outside this container
EXPOSE 80

# Run DoomClone when the container launches
CMD ["./DoomClone"]