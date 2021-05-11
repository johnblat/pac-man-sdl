CC=clang

OBJS = ./src/pac_game.c ./src/actor.c ./src/animation.c ./src/render.c ./src/resources.c ./src/targeting.c ./src/tiles.c ./src/movement.c ./src/states.c
TILEMAP_OBJS = ./src/tilemap_editor.c

COMPILER_FLAGS = -w 

SDL_INC = -I/usr/include/SDL2 -Iinclude/

SDL_LIBS = -lSDL2 -lSDL2main -lm  -lSDL2_image -lSDL2_ttf


EXE = ./bin/pac_game
TM_EXE = ./bin/tilemap_editor

tilemap: $(TILEMAP_OBJS)
	$(CC) $(TILEMAP_OBJS) -o $(TM_EXE) $(SDL_INC) $(SDL_LIBS)
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) -o $(EXE) $(SDL_INC) $(SDL_LIBS) 