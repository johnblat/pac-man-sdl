#!/usr/bin/bash



# compile
echo "compiling...."
clang src/pac_game_jumbo.c -o linux/builds/PacManSDL.AppDir/usr/bin/pac-man-sdl -I/usr/include/SDL2 -Iinclude/ -lSDL2 -lSDL2main -lm -lSDL2_image -lSDL2_ttf -lSDL2_mixer

# sync
echo "rsyncing data files into AppDir"
rsync -r --delete res/ linux/builds/PacManSDL.AppDir/res 

# linuxdeploy to gather any dependencies
linuxdeploy --appdir=linux/builds/PacManSDL.AppDir 

# create appimage
export ARCH=x86_64
TODAY=$(date +"%Y-%m-%d")
appimagetool -v linux/builds/PacManSDL.AppDir linux/builds/AppImages/Pac-Man-SDL-${TODAY}-x86_64.AppImage
