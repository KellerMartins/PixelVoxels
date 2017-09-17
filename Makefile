PD = C:
#CC specifies which compiler we're using
CC = gcc

#OBJS specifies which files to compile as part of the project
OBJS = main.c voxelLoader.c voxelRenderer.c  voxelLogic.c utils.c

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -I $(PD)\SDL2\SDL2_MinGW_32Bits\include -I $(PD)\SoLoud\include 

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -L $(PD)\SDL2\SDL2_MinGW_32Bits\lib -I $(PD)\SoLoud\lib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
# -Wl,-subsystem,windows
# -fopenmp enables openmp support
COMPILER_FLAGS = -Wall -ffast-math -O3

#LINKER_FLAGS specifies the libraries we're linking against -lglew32  -mwindows
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lpthread $(PD)\SoLoud\lib\soloud_x86.lib

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = Space

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)