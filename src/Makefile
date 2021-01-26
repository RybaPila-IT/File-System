G++   := g++
FLAGS := -std=c++14 -pedantic -Wall -Werror

HEADERS := allocator.h file_system.h inodes.h memory_blocks.h utility.h
MAIN    := main.cpp

.PHONY: all clean move

all: main

main: $(MAIN) $(HEADERS)
	$(G++) $(FLAGS) $(MAIN) -o main
	
clean:
	rm -f main ../build/main  
	
move:
	mkdir ../build
	mv main ../build

