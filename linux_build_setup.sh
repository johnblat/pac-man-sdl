#!/usr/bin/bash

# appImage builds
mkdir -r linux/builds/AppImages 
# appDir to make AppImages with
mkdir -r linux/builds/PacManSDL.AppDir
# linuxdeploy to create initial AppDir
# should create usr, usr/lib, usr/bin and others
linuxdeploy --appdir=linux/builds/PacManSDL.AppDir 
# sync res
rsync -r --delete res/ linux/builds/PacManSDL.AppDir/res 
# create AppRun
touch linux/builds/PacManSDL.AppDir/AppRun
cat > linux/builds/PacManSDL.AppDir/AppRun << EOL
#!/bin/bash
echo "script start"
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export PATH="${HERE}/usr/bin/:${HERE}/usr/sbin/:${HERE}/usr/games/:${HERE}/bin/:${HERE}/sbin/${PATH:+:$PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib/:${HERE}/usr/lib/i386-linux-gnu/:${HERE}/usr/lib/x86_64-linux-gnu/:${HERE}/usr/lib32/:${HERE}/usr/lib64/:${HERE}/lib/:${HERE}/lib/i386-linux-gnu/:${HERE}/lib/x86_64-linux-gnu/:${HERE}/lib32/:${HERE}/lib64/${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export XDG_DATA_DIRS="${HERE}/usr/share/${XDG_DATA_DIRS:+:$XDG_DATA_DIRS}:${HERE}/res"
export GSETTINGS_SCHEMA_DIR="${HERE}/usr/share/glib-2.0/schemas/${GSETTINGS_SCHEMA_DIR:+:$GSETTINGS_SCHEMA_DIR}"
EXEC=$(grep -e '^Exec=.*' "${HERE}"/*.desktop | head -n 1 | cut -d "=" -f 2 | cut -d " " -f 1)
echo ${EXEC}
echo "${HERE}"
cd ${HERE}
exec "${HERE}/usr/bin/pac-man-sdl"
EOL

chmod a+x linux/builds/PacManSDL.AppDir/AppRun

# create desktop file
touch linux/builds/PacManSDL.AppDir/pac-man-sdl.dektop 
cat > linux/builds/PacManSDL.AppDir/pac-man-sdl.dektop << EOL
[Desktop Entry]
Name=Pac-Man SDL
Exec=usr/bin/pac-man-sdl
Icon=Pacman_Thumbnail
Type=Application
Comment="Inspired By Pac-Man Arrangement"
Categories=Game;
EOL

# cp icon
cp res/img/Pacman_Thumbnail.png linux/builds/PacManSDL.AppDir/Pacman_Thumbnail.png

