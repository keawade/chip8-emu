/*
http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 */
#include "chip8.h"
#include <iostream>

using namespace std;

Chip8 emulator;

int main(int argc, char **argv)
{

    // setupGraphics();
    // setupInput();

    // Initalize Chip-8 system
    emulator.initialize();

    // Load program into memory
    emulator.loadProgram("programs/IBM.ch8");

    for (;;)
    {
        // Emulate a cycle
        emulator.emulateCycle();

        // if (emulator.drawFlag)
        //   drawGraphics();

        // emulator.setKeys();
    }
    return 0;
}
