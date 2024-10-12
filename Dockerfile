# Use an official Ubuntu as a parent image
FROM ubuntu:latest

# Set the working directory
WORKDIR /usr/src/app

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-ttf-dev \
    wget \
    unzip

# Copy the current directory contents into the container at /usr/src/app
COPY . .

# Compile the game
RUN g++ -Iinclude -Iinclude/sdl -Iinclude/headers -Llib -o DoomClone src/*.cpp -lmingw32 -lSDL2 -lSDL2_image -lSDL2_ttf -mconsole

# Make port 80 available to the world outside this container
EXPOSE 80

# Run DoomClone when the container launches
CMD ["./DoomClone"]