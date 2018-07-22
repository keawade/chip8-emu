CC=gcc

chip8make: main.cpp chip8.cpp
	$(CC) -o output main.cpp chip8.cpp -I. -lncurses -lstdc++