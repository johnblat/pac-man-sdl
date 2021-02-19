CC=clang

OBJS = ./src/pac_game.c 
TILEMAP_OBJS = ./src/tilemap_editor.c

COMPILER_FLAGS = -w -O0

SDL_INC = -I/usr/include/SDL2

SDL_LIBS = -lSDL2 -lSDL2main -lm  -lSDL2_image


EXE = ./bin/pac_game
TM_EXE = ./bin/tilemap_editor

tilemap: $(TILEMAP_OBJS)
	$(CC) $(TILEMAP_OBJS) -o $(TM_EXE) $(SDL_INC) $(SDL_LIBS)
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) -o $(EXE) $(SDL_INC) $(SDL_LIBS) 