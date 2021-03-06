PD = C:
#CC specifies which compiler we're using
CC = gcc

#SRC specifies which files to compile as part of the project
SRC = $(wildcard Source/*.c) $(wildcard Source/Engine/*.c) $(wildcard Source/Components/*.c) $(wildcard Source/Systems/*.c) $(wildcard Source/Systems/Editor/*.c) $(wildcard Source/Libs/*.c) 
OBJS = $(SRC:%.c=Build/%.o)

#FOLDERS specifies the folders structure needed to put the .o files
ifeq ($(OS),Windows_NT)
FOLDERS = Build Build\Source Build\Source\Engine Build\Source\Components Build\Source\Systems Build\Source\Systems\Editor Build\Source\Libs 
else
FOLDERS = Build Build/Source Build/Source/Engine Build/Source/Components Build/Source/Systems Build/Source/Systems/Editor Build/Source/Libs
endif

#COMPILER_FLAGS specifies the additional compilation options we're using
COMPILER_FLAGS += -Wall -Wno-unused-result -Wno-missing-braces -ffast-math -O3 -g

#LINKER_FLAGS specifies the libraries we're linking against
#Both LINKER_FLAGS and MKDIR are defined based on the OS
ifeq ($(OS),Windows_NT)
#Include and Library paths needed on Windows
INCLUDE_PATHS = -I $(PD) -I $(PD)\SDL2\include -I $(PD)\glew\include -I $(PD)\lua5.3\include
LIBRARY_PATHS = -L $(PD)\SDL2\lib -L $(PD)\glew\lib -L $(PD)\lua5.3\lib
LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lglew32.dll -llua53 -lopengl32 Source/resource.res

FixPath = $(subst /,\,$1)
MakeDir = if not exist "$(call FixPath,$1)" mkdir $(call FixPath,$1)
RemoveDir = rd /s /q $(call FixPath,$1)
RemoveFile = del /q $(call FixPath,$1)
EchoNewline = @echo. && echo.$1

else
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lGLEW -lGL -llua5.3 -lm

FixPath = $1
MakeDir = mkdir -p $1
RemoveDir = rm -f -r $1
RemoveFile = rm -f $1
EchoNewline = @echo ""; echo "$1"

endif

#Name of the executable
OBJ_NAME = Vopix

#Compilation Targets
all : $(OBJ_NAME) 
	@echo Build finished!

$(OBJ_NAME): $(FOLDERS)/ $(OBJS) 
	$(call EchoNewline,+ Compiling program "$@")
ifeq ($(OS),Windows_NT)
	@$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
else
	@$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
endif

Build/%.o: %.c %.h
	$(call EchoNewline,- Compiling object "$@")
ifeq ($(OS),Windows_NT)
	@$(CC) -c $(INCLUDE_PATHS) $(COMPILER_FLAGS) $< -o $@
else
	@$(CC) -c $(COMPILER_FLAGS) $< -o $@
endif

Build/%.o: %.c
	$(call EchoNewline,- Compiling object "$@")
ifeq ($(OS),Windows_NT)
	@$(CC) -c $(INCLUDE_PATHS) $(COMPILER_FLAGS) $< -o $@
else
	@$(CC) -c $(COMPILER_FLAGS) $< -o $@
endif

$(FOLDERS)/:
	$(call EchoNewline,+ Creating build folders "$@")
	$(call MakeDir,$@)

clear:
	-$(call RemoveDir,Build)

tcc:
	tcc $(SRC) -D SDL_DISABLE_IMMINTRIN_H $(COMPILER_FLAGS) $(LINKER_FLAGS) -run
	
