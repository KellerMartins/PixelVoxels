PD = C:
#CC specifies which compiler we're using
CC = gcc

#SRC specifies which files to compile as part of the project
SRC = $(wildcard Source/*.c) $(wildcard Source/Components/*.c) $(wildcard Source/Systems/*.c) $(wildcard Source/Libs/*.c) 
OBJS = $(SRC:%.c=Build/%.o)

#FOLDERS specifies the folders structure needed to put the .o files
FOLDERS = Build Build/Source Build/Source/Components Build/Source/Systems Build/Source/Libs

#COMPILER_FLAGS specifies the additional compilation options we're using
COMPILER_FLAGS = -Wall -Wno-unused-result -Wno-missing-braces -ffast-math -O3

#LINKER_FLAGS specifies the libraries we're linking against
#Both LINKER_FLAGS and MKDIR are defined based on the OS
ifeq ($(OS),Windows_NT)
#Include and Library paths needed on Windows
INCLUDE_PATHS = -I $(PD)\SDL2\SDL2_MinGW_32Bits\include -I $(PD)\SoLoud\include -I $(PD)\glew\include -I $(PD)\lua\include
LIBRARY_PATHS = -L $(PD)\SDL2\SDL2_MinGW_32Bits\lib -L $(PD)\SoLoud\lib -L $(PD)\glew\lib -L $(PD)\lua\lib
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lglew32.dll -llua53 -lopengl32 $(PD)\SoLoud\lib\soloud_x86.lib Source/resource.res
MKDIR = @mkdir

else
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lGLEW -lGL -lm 
MKDIR = @mkdir -p

endif

#Name of the executable
OBJ_NAME = Space

#Compilation Targets
all : $(OBJ_NAME) 

$(OBJ_NAME): $(FOLDERS)/ $(OBJS) 
	@echo "+ Compiling program \"$@\""
	@$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

Build/%.o: %.c %.h
	@echo "- Compiling object \"$@\""
	@$(CC) -c $(COMPILER_FLAGS) $< -o $@

Build/%.o: %.c
	@echo "- Compiling object \"$@\""
	@$(CC) -c $(COMPILER_FLAGS) $< -o $@

$(FOLDERS)/:
	@echo "+ Creating build folders \"$@\""
	$(MKDIR) $@
