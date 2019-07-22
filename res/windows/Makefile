#usage mingw32-make clean, mingw32-make

# Files to be compiled
SRC = C:\snibbetracker\src\CAllocator.c C:\snibbetracker\src\CEngine.c C:\snibbetracker\src\CInput.c C:\snibbetracker\src\CSynth.c C:\snibbetracker\src\cJSON\cJSON.c C:\snibbetracker\src\dir_win.c C:\snibbetracker\src\main.c
OBJ=$(SRC:.c=.o) # replaces the .c from SRC with .o

# Compiler
CC = gcc

RM=del

# Include paths
INCLUDE_PATHS = -IC:\Downloads\SDL2-devel-2.0.4-mingw\i686-w64-mingw32\include -IC:\snibbetracker\src\cJSON

# Library paths
LIBRARY_PATHS = -LC:\Downloads\SDL2-devel-2.0.4-mingw\i686-w64-mingw32\lib

# Compiler flags
COMPILER_FLAGS = -Wall -std=c99 -Wno-unused-function

# Linker flags
LINKER_FLAGS = -static -lmingw32 -lSDL2main -lSDL2 -mwindows -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc

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
	$(CC) $(OBJ) my.res $(LIBRARY_PATHS) $(LINKER_FLAGS) -o $@

.PHONY : clean   # .PHONY ignores files named clean
clean:
	-$(RM) $(OBJ) $(EXE)