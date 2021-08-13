Pac-Man SDL
====================

Concept
-------

Inspired by Pac-Man Arrangment.
Local Multiplayer with friends.

Controls
--------

**Keyboard**
- Arrow Keys to move
- Hold Z to charge dash
    - Release to dash
- Press B to disable Debug Visuals
- Enter/Return = OK

**Controller**
- D-Pad to move
- Hold X (XBox) to charge dash
- A = OK

Editing and Modifying
=======

Editing of the game is done in 2 main ways:
- Editing via the **tilemap editor** GUI
- Editing via text config files

As of right now, both are required for editing.

Scopes:
- Level: Configures individual level
- Global: Configures entire game

### Tilemap Editor
Scope: Level

**Controls**

- S: **S**ave
- L: **L**oad
- W: **W**all placement mode
- T: **T**ile placement mode
- G: **G**host Pen placement mode
- P: **P**ac-Man Starting tile placement mode
- D: Pac-**D**ot placement mode
- E: Power Pellet (**E**nergizer pellet) placement mode. Max 4.
- V: **V**ery Slow tile placement mode ( Ghosts move slow on these tiles ). Max 20
- Arrow keys: Move map
- LMB: Select tile from tile selection panel in Tile Mode. Place object on tilemap ( depends on mode ).
- RMB: Remove object from tilemap ( depends on mode ).

**Level Selection**

Edit the `res/tilemap_editor_level` file to contain the level number to edit. Whatever number is in this file is the level that the tilemap editor will edit upon launching. To edit a different level, close the program, edit the file, save, and then relaunch. 

Create the corresponsding `res/levels/level#` directory of the level to edit if the level does not exist yet. Substitute `#` for the level number

### Text file editing

**Animated Sprites**
- Scope: Global
- File: `res/animated_sprites`

**Default Speed**
- Scope: Global
- File: `res/default_pac_speed`
- Controls the 100% speed of entities in the game. ( speed multiplier of x1)
    - Pac-Man is set to x1 ( 100% )
    - Ghosts are normally at x0.85 ( 85% )
    - Vulnerable ghosts are x0.60
    - Ghosts travelling through tunnels are x0.60
    - Other factors can effect the speed multiplier 
- Think of this as a "base speed" that mulitpliers are added to for various factors and gameplay elements
- Adjusting this will change the speed and flow of the game

**Ghost Mode Times**
- Scope: Level
- File: `res/levels/level#/ghost_mode_times`
- For every level, you can set how long (in seconds) the ghosts should remain in their scatter or chase periods
- There are a total of nine periods
- Scatter, chase, scatter, chase, ... and so on
- Seperate period lengths by spaces ` ` 
- Example: `7 20 7 20 7 20 5 5 5`

**Global Texture Atlases**
- Scope: Global
- File: `res/texture_atlases`
- These are global texture atlases ( AKA sprite sheets ) that are available by any file including `render.h`
- These are texture atlases intended to be used throughout the entire game.
- The rows and cols should be specified so that the sprite clips will be broken up appropriately
- At each texture atlas specified, it will be assigned an incrementing ID from `0, 1, 2, ... ` and so on. This ID is to be referenced in other places where specifying what texture atlas to use. For example, in `res/animated_sprites`. By specifying the ID here, you are telling the animated sprite to point to that texture atlas.

Building
========

Linux:
- For development, one can use the Makefile
- Required libs:
    - libSDL2, libSDL2_image, libSDL2_ttf, libSDL2_mixer (install through your distribution's package manager)
- Run the linux scripts for building a distributable version. The chosen file format is AppImage. 
- Required tools for scripts provided (For building a distributable):
    - [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy)
        - Helps build the AppDir and gather library dependencies from your system
    - [AppImageToolkit](https://github.com/AppImage/AppImageKit)
        - Converts the AppDir into an AppImage
    
Windows:
- Required libs:
    - libSDL2, libSDL2_image, libSDL2_ttf, libSDL2_mixer (install from whatever website hosts these for windows builds)
- Can use the windows scripts to build a distributable



Credits
=======

- Programming: 
    - John Blat 
- Art:
    - Stephen Bath
    - Colton Powell
- Music:
    - **Rock Paper Funk Take Off** by Broken Groove
    - **Don't Worry We've Got Warp Speed** by Broken Groove
    - **Trust Relax Autopilot** by Broken Groove
    - **DisFunkTional** by ThatAndyGuy
    - **Optionstest** ( Main menu music ) by Colonel Sandwich 

