PD = C:
#CC specifies which compiler we're using
CC = gcc

#SRC specifies which files to compile as part of the project
SRC = $(wildcard Source/*.c) $(wildcard Source/Components/*.c) $(wildcard Source/Systems/*.c) $(wildcard Source/Systems/Editor/*.c) $(wildcard Source/Libs/*.c) 
OBJS = $(SRC:%.c=Build/%.o)

#FOLDERS specifies the folders structure needed to put the .o files
FOLDERS = Build Build/Source Build/Source/Components Build/Source/Systems Build/Source/Systems/Editor Build/Source/Libs

#COMPILER_FLAGS specifies the additional compilation options we're using
COMPILER_FLAGS = -Wall -Wno-unused-result -Wno-missing-braces -ffast-math -O3 -g

#LINKER_FLAGS specifies the libraries we're linking against
#Both LINKER_FLAGS and MKDIR are defined based on the OS
ifeq ($(OS),Windows_NT)
#Include and Library paths needed on Windows
INCLUDE_PATHS = -I $(PD)\SDL2\SDL2_MinGW_32Bits\include -I $(PD)\SoLoud\include -I $(PD)\glew\include -I $(PD)\lua5.3\include
LIBRARY_PATHS = -L $(PD)\SDL2\SDL2_MinGW_32Bits\lib -L $(PD)\SoLoud\lib -L $(PD)\glew\lib -L $(PD)\lua5.3\lib
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lglew32.dll -llua53 -lopengl32 $(PD)\SoLoud\lib\soloud_x86.lib Source/resource.res
MKDIR = @mkdir

else
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lGLEW -lGL -llua5.3 -lm
MKDIR = @mkdir -p

endif

#Name of the executable
OBJ_NAME = Space

#Compilation Targets
all : $(OBJ_NAME) 


$(OBJ_NAME): $(FOLDERS)/ $(OBJS) 
	@echo "+ Compiling program \"$@\""
ifeq ($(OS),Windows_NT)
	@$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
else
	@$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
endif

Build/%.o: %.c %.h
	@echo "- Compiling object \"$@\""
ifeq ($(OS),Windows_NT)
	@$(CC) -c $(INCLUDE_PATHS) $(COMPILER_FLAGS) $< -o $@
else
	@$(CC) -c $(COMPILER_FLAGS) $< -o $@
endif

Build/%.o: %.c
	@echo "- Compiling object \"$@\""
ifeq ($(OS),Windows_NT)
	@$(CC) -c $(INCLUDE_PATHS) $(COMPILER_FLAGS) $< -o $@
else
	@$(CC) -c $(COMPILER_FLAGS) $< -o $@
endif

$(FOLDERS)/:
	@echo "+ Creating build folders \"$@\""
	$(MKDIR) $@
