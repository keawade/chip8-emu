/*
http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 */
#include <iostream>
#include <ncurses.h>
#include <thread>
#include <chrono>

#include "chip8.h"

using namespace std;

Chip8 emulator;

int main(int argc, char **argv)
{
    // Start curses mode
    initscr();

    // setupInput();

    // Initalize Chip-8 system
    emulator.initialize();

    // Load program into memory
    emulator.loadProgram("programs/pong2.c8");

    for (;;)
    {
        // Emulate a cycle
        emulator.emulateCycle();

        // If we're supposed to draw this cycle
        if (emulator.drawFlag)
        {
            // Loop through all the pixels
            for (uint y = 0; y < 32; ++y)
            {
                for (uint x = 0; x < 64; ++x)
                {
                    // Move the ncurses cursor to that pixel
                    move(y, x);

                    // If that pixel is "on"
                    if (emulator.gfx[y * 64 + x])
                    {
                        // Add the checkboard character to that position
                        addch(ACS_CKBOARD);
                    }
                }
            }

            // Flip the draw flag
            emulator.drawFlag = false;
        }

        // Print ncurses view to the terminal
        refresh();

        // emulator.setKeys();

        this_thread::sleep_for(chrono::milliseconds(10));
    }

    // End curses mode
    endwin();

    return 0;
}
