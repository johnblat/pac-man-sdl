CC=clang

OBJS = ./src/pac_game_jumbo.c
TILEMAP_OBJS = ./src/tilemap_editor.c

COMPILER_FLAGS = -w -Wall -Werror -g -fno-omit-frame-pointer -std=c99

SDL_INC = -I/usr/include/SDL2 -Iinclude/

SDL_LIBS = -lSDL2 -lSDL2main -lm  -lSDL2_image -lSDL2_ttf -lSDL2_mixer


EXE = ./bin/pac_game
TM_EXE = ./bin/tilemap_editor

tilemap: $(TILEMAP_OBJS)
	$(CC) $(TILEMAP_OBJS) -o $(TM_EXE) $(SDL_INC) $(SDL_LIBS)
game : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) -o $(EXE) $(SDL_INC) $(SDL_LIBS) 