PD = C:
#CC specifies which compiler we're using
CC = gcc

#OBJS specifies which files to compile as part of the project
OBJS = Source/main.c Source/Systems/voxelLoader.c Source/Systems/voxelRenderer.c  Source/Systems/voxelLogic.c Source/utils.c Source/Engine.c Source/Components/voxelModel.c 

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -I $(PD)\SDL2\SDL2_MinGW_32Bits\include -I $(PD)\SoLoud\include -I $(PD)\glew\include

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -L $(PD)\SDL2\SDL2_MinGW_32Bits\lib -L $(PD)\SoLoud\lib -L $(PD)\glew\lib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
# -Wl,-subsystem,windows
# -fopenmp enables openmp support
COMPILER_FLAGS = -Wall -Wno-unused-result -Wno-missing-braces -ffast-math -O3

#LINKER_FLAGS specifies the libraries we're linking against -mwindows
LINKER_FLAGS_W = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lglew32.dll -lopengl32 $(PD)\SoLoud\lib\soloud_x86.lib Source/resource.res
LINKER_FLAGS_L = -lSDL2 -lSDL2_image -lSDL2_ttf -lGLEW -lGL -lm 

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = Space

#This is the target that compiles our executable
ifeq ($(OS),Windows_NT)
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS_W) -o $(OBJ_NAME)
else
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS_L) -o $(OBJ_NAME)
endif
