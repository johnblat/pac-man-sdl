#!/usr/bin/bash

# cross-compile
echo "compiling..."
x86_64-w64-mingw32-gcc -std=c99 src/pac_game_jumbo.c -Iinclude -Lwindows/lib -o windows/PacGame/pac-man-sdl -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
x86_64-w64-mingw32-gcc -std=c99 src/editor_jumbo.c -Iinclude -Lwindows/lib -o windows/PacGame/pac-tilemap_editor-win -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

# sync up the data files
echo "syncing data files res"
rsync res windows/PacGame/res

# zip it up
echo "zipping..."
TODAY=$(date +"%Y-%m-%d")
cd windows
zip -r builds/PacManSDL-${TODAY}.zip PacGame 
