PD = C:
CC = gcc

OBJS = main.c voxelLoader.c voxelRenderer.c  voxelLogic.c utils.c SDL_FontCache.c

COMPILER_FLAGS = -Wno-unused-result -std=c99 -Wall -ffast-math -O3 -g
LINKER_FLAGS = -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lpthread -lm 

OBJ_NAME = Space

all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)