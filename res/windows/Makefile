# Files to be compiled
SRC = C:\Documents\cengine\CAllocator.c C:\Documents\cengine\CAnimation.c C:\Documents\cengine\CAnimationFrame.c C:\Documents\cengine\CButton.c C:\Documents\cengine\CEngine.c C:\Documents\cengine\CEntity.c C:\Documents\cengine\CInput.c C:\Documents\cengine\CLabel.c C:\Documents\cengine\CListQueue.c C:\Documents\cengine\CSynth.c C:\Documents\cengine\CTimer.c C:\Documents\cengine\CTouch.c C:\Documents\cengine\cJSON\cJSON.c C:\Documents\csynth\SDL2_template\main.c
OBJ=$(SRC:.c=.o) # replaces the .c from SRC with .o

# Compiler
CC = gcc

RM=rm

# Include paths
INCLUDE_PATHS = -IC:\Downloads\SDL2-devel-2.0.3-mingw\i686-w64-mingw32\include -IC:\Documents\cengine -IC:\Documents\cengine\cJSON -IC:\Documents\csynth\SDL2_template
#INCLUDE_PATHS = -IC:\Downloads\SDL2-devel-2.0.3-mingw\x86_64-w64-mingw32\include -IC:\Documents\cengine -IC:\Documents\cengine\cJSON -IC:\Documents\csynth\SDL2_template


# Library paths
LIBRARY_PATHS = -LC:\Downloads\SDL2-devel-2.0.3-mingw\i686-w64-mingw32\lib
#LIBRARY_PATHS = -LC:\Downloads\SDL2-devel-2.0.3-mingw\x86_64-w64-mingw32\lib

# Compiler flags
COMPILER_FLAGS = -Wall -std=c99

# Linker flags
#LINKER_FLAGS = -static -lmingw32 -lSDL2main -lSDL2 -mwindows -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc

LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2

# Executable file
EXE = snibbetracker.exe

# Compile command
#all: 
#	$(SRC) 
#	$(CC) $(SRC) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(EXE)

%.o: %.c         # combined w/ next line will compile recently changed .c files
	$(CC) $(COMPILER_FLAGS) $(INCLUDE_PATHS) -o $@ -c $<
	
.PHONY : all     # .PHONY ignores files named all
all: $(EXE)      # all is dependent on $(EXE) to be complete

$(EXE): $(OBJ)   # $(EXE) is dependent on all of the files in $(OBJ) to exist
	$(CC) $(OBJ) $(LIBRARY_PATHS) $(LINKER_FLAGS) -o $@

.PHONY : clean   # .PHONY ignores files named clean
clean:
	-$(RM) $(OBJ) core    # '-' causes errors not to exit the process