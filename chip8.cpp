#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "chip8.h"

using namespace std;

Chip8::Chip8()
{
    cycle = 0; // Set cycle tracker to 0 (Only used for debugging)
}

Chip8::~Chip8()
{
    cycle = 0; // Set cycle tracker to 0 (Only used for debugging)
}

int16_t chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8::clearKeys()
{
    for (int i = 0; i < 16; i++)
        key[i] = 0;
}

void Chip8::setKey(char k)
{
    //    1 2 3 C    ->    1 2 3 4
    //    4 5 6 D    ->    Q W E R
    //    7 8 9 E    ->    A S D F
    //    0 A B F    ->    Z X C V

    // clearKeys();

    switch (k)
    {
    // 0
    case 'z':
        key[0x0] = 1;
        break;
    // 1
    case '1':
        key[0x1] = 1;
        break;
    // 2
    case '2':
        key[0x2] = 1;
        break;
    // 3
    case '3':
        key[0x3] = 1;
        break;
    // 4
    case 'q':
        key[0x4] = 1;
        break;
    // 5
    case 'w':
        key[0x5] = 1;
        break;
    // 6
    case 'e':
        key[0x6] = 1;
        break;
    // 7
    case 'a':
        key[0x7] = 1;
        break;
    // 8
    case 's':
        key[0x8] = 1;
        break;
    // 9
    case 'd':
        key[0x9] = 1;
        break;
    // a
    case 'x':
        key[0xA] = 1;
        break;
    // b
    case 'c':
        key[0xB] = 1;
        break;
    // c
    case '4':
        key[0xC] = 1;
        break;
    // d
    case 'r':
        key[0xD] = 1;
        break;
    // e
    case 'f':
        key[0xE] = 1;
        break;
    // f
    case 'v':
        key[0xF] = 1;
        break;
    default:
        // do nothing
        break;
    }
}

void log(int cycle, string severity, string fnctn, string message)
{
    // ofstream logfile;
    // logfile.open("debug.log", ios::app);
    // string c = to_string(cycle);
    // c.insert(c.begin(), 8 - c.length(), '0');
    // logfile << c + " " + severity + ": " + fnctn + " - " + message << endl;
}

void Chip8::initialize()
{
    log(cycle, "LOG", "Chip8::initialize", "initializing");

    pc = 0x200; // Program counter starts at 0x200
    opcode = 0; // Reset current opcode
    I = 0;      // Reset index register
    sp = 0;     // Reset stack pointer

    // initialize clear display
    for (uint i = 0; i < 2048; ++i)
        gfx[i] = 0;

    // initialize clear stack
    for (uint i = 0; i < 16; ++i)
        stack[i] = 0;

    // initialize clear registers V0-VF
    for (uint i = 0; i < 16; ++i)
        V[i] = 0;

    // initialize clear keyboard state
    clearKeys();

    // initialize clear memory
    for (uint i = 0; i < 4096; ++i)
        memory[i] = 0;

    // load font set
    for (uint i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];

    // reset timers
    delay_timer = 0;
    sound_timer = 0;
}

template <typename T>
std::string int_to_hex(T i)
{
    // Source: https://stackoverflow.com/a/5100745
    std::stringstream stream;
    stream << "0x"
           << std::setfill('0') << std::setw(sizeof(T) * 2)
           << std::hex << i;
    return stream.str();
}

void Chip8::emulateCycle()
{
    cycle++;

    // fetch opcode
    // Store both bytes of the opcode in a single two byte variable
    opcode = memory[pc] << 8 | memory[pc + 1];

    log(cycle, "LOG", "Chip8::emulateCycle", int_to_hex(opcode));

    // decode and execute opcode
    switch (A)
    {
    case 0x0:
    {
        switch (low)
        {
        case 0xE0:
        {
            // 00E0 - CLS
            // Clear the display.
            for (uint i = 0; i < 2048; ++i)
                gfx[i] = 0;
            drawFlag = true;
            pc += 2;
            break;
        }
        case 0xEE:
        {
            // 00EE - RET
            // Return from a subroutine.
            --sp;
            pc = stack[sp];
            pc += 2;
            break;
        }
        default:
            // printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            log(cycle, "ERR", "Chip8::emulateCycle", "Unknown opcode [0x0000]: 0x" + opcode);
            pc += 2;
        }
        break;
    }

    case 0x1: // 1nnn: JP addr - set program counter to nnn
    {
        pc = NNN; // Jump to nnn
        break;
    }

    case 0x2:
    {
        // 2nnn - CALL addr
        // Call subroutine at nnn.
        stack[sp] = pc; // Store current program counter
        ++sp;           // Bump the stack pointer
        pc = NNN;       // Jump to nnn
        break;
    }

    case 0x3:
    {
        // 3xkk - SE Vx, byte
        // Skip next instruction if Vx = kk.
        if (V[X] == low)
        {
            pc += 2;
        }
        pc += 2;
        break;
    }
    case 0x4:
    {
        // 4xkk - SNE Vx, byte
        // Skip next instruction if Vx != kk.
        if (V[X] != low)
        {
            pc += 2;
        }
        pc += 2;
        break;
    }
    case 0x5:
    {
        // 5xy0 - SE Vx, Vy
        // Skip next instruction if Vx = Vy.
        if (V[X] == V[Y])
        {
            pc += 2;
        }
        pc += 2;
        break;
    }

    case 0x6:
    {
        // 6xkk - LD Vx, byte
        // Set Vx = kk.
        V[X] = low;
        pc += 2;
        break;
    }
    case 0x7:
    {
        // 7xkk - ADD Vx, byte
        // Set Vx = Vx + kk.
        V[X] += low;
        pc += 2;
        break;
    }
    case 0x8:
    {
        switch (N)
        {
        case 0x0:
        {
            // 8xy0 - LD Vx, Vy
            // Set Vx = Vy.
            V[X] = V[Y];
            pc += 2;
            break;
        }
        case 0x1:
        {
            // 8xy1 - OR Vx, Vy
            // Set Vx = Vx OR Vy.
            V[X] = V[X] | V[Y];
            pc += 2;
            break;
        }
        case 0x2:
        {
            // 8xy2 - AND Vx, Vy
            // Set Vx = Vx AND Vy.
            V[X] = V[X] & V[Y];
            pc += 2;
            break;
        }
        case 0x3:
        {
            // 8xy3 - XOR Vx, Vy
            // Set Vx = Vx XOR Vy.
            V[X] = V[X] ^ V[Y];
            pc += 2;
            break;
        }
        case 0x4:
        {
            // 8xy4 - ADD Vx, Vy
            // Set Vx = Vx + Vy, set VF = carry.
            V[X] += V[Y];

            if (V[X] > V[Y])
                V[0xF] = 1; // carry
            else
                V[0xF] = 0;

            pc += 2;
            break;
        }
        case 0x5:
        {
            // 8xy5 - SUB Vx, Vy
            // Set Vx = Vx - Vy, set VF = NOT borrow.
            if (V[X] > V[Y])
                V[0xF] = 1;
            else
                V[0xF] = 0;

            V[X] -= V[Y];
            pc += 2;
            break;
        }
        case 0x6:
        {
            // 8xy6 - SHR Vx {, Vy}
            // Set Vx = Vx SHR 1.
            // Shifts VY right by one and stores the result to VX (VY remains unchanged). VF is set to the value of the least significant bit of VY before the shift.

            // Apparently a lot of modern interpreters ignore the docs for this and do this way:
            V[0xF] = V[X] & 1;
            V[X] /= 2;

            // // "Correct" way:
            // // Store LSD of Vy in VF
            // V[0xF] = V[Y] & 1;
            // // Store Vy >> 1 in Vx
            // V[X] = V[Y] >> 1;

            pc += 2;
            break;
        }
        case 0x7:
        {
            // 8xy7 - SUBN Vx, Vy
            // Set Vx = Vy - Vx, set VF = NOT borrow.
            if (V[X] > V[Y])
                V[0xF] = 1;
            else
                V[0xF] = 0;

            V[X] = V[Y] - V[X];
            pc += 2;
            break;
        }
        case 0xE:
        {
            // 8xyE - SHL Vx {, Vy}
            // Set Vy = Vx = Vy SHL 1.
            // Shifts VY left by one and copies the result to VX. VF is set to the value of the most significant bit of VY before the shift.

            // Store MSD of Vy in VF
            V[0xF] = V[Y] >> 7;

            // Again, modern interpreters ignore the docs so... :(
            V[X] = V[X] * 2;

            // // "Correct" way:
            // // Store Vy << 1 in Vx
            // V[X] = V[Y] << 1;
            // // Store Vy << 1 in Vy
            // V[Y] = V[Y] << 1;
            pc += 2;
            break;
        }
        default:
        {
            // printf("Unknown opcode [0x8000]: 8x%X\n", opcode);
            log(cycle, "ERR", "Chip8::emulateCycle", "Unknown opcode [0x8000]: 8x" + opcode);
            pc += 2;
            break;
        }
        }
        break;
    }

    case 0x9:
    {
        // 9xy0 - SNE Vx, Vy
        // Skip next instruction if Vx != Vy.
        if (V[X] != V[Y])
        {
            pc += 2;
        }
        pc += 2;
        break;
    }
    case 0xA:
    {
        // Annn - LD I, addr
        // Set I = nnn.
        I = NNN;
        pc += 2;
        break;
    }
    case 0xB:
    {
        // Bnnn - JP V0, addr
        // Jump to location nnn + V0.
        pc = NNN + V[0];
        break;
    }
    case 0xC:
    {
        // Cxkk - RND Vx, byte
        // Set Vx = random byte AND kk.
        V[X] = (rand() % 0xFF) & low;
        pc += 2;
        break;
    }
    case 0xD:
    {
        // Dxyn - DRW Vx, Vy, nibble
        // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        u_int8_t pixel;

        // Reset VF. We'll set this to 1 later if there is a collision.
        V[0xF] = 0;

        // For every line on the y axis
        for (uint yline = 0; yline < N; yline++)
        {
            // Get the pixel data from memory
            pixel = memory[I + yline];
            for (uint xline = 0; xline < 8; xline++)
            {
                // 0x80 == 0b10000000
                // For each x value, check if it is to be toggled (pixel AND current value from mem != 0)
                if ((pixel & (0x80 >> xline)) != 0)
                {
                    // If a pixel is to be toggled
                    // Check if if the value is already on
                    if (gfx[(V[X] + xline + ((V[Y] + yline) * 64))] == 1)
                    {
                        // If it is already on, set VF to 1
                        V[0xF] = 1;
                    }
                    // XOR the given value with the new value
                    gfx[V[X] + xline + ((V[Y] + yline) * 64)] ^= 1;
                }
            }
        }

        // Set the drawFlag to tell the system to update the screen
        drawFlag = true;

        pc += 2;
        break;
    }

    case 0xE:
    {
        switch (low)
        {
        case 0x9E:
        {
            // Ex9E - SKP Vx
            // Skip next instruction if key with the value of Vx is pressed.
            if (key[V[X]] != 0)
            {
                pc += 2;
            }
            pc += 2;
            break;
        }
        case 0xA1:
        {
            // ExA1 - SKNP Vx
            // Skip next instruction if key with the value of Vx is not pressed.
            if (key[V[X]] == 0)
            {
                pc += 2;
            }
            pc += 2;
            break;
        }
        default:
        {
            // printf("Unknown opcode [0xE000]: Ex%X\n", opcode);
            log(cycle, "ERR", "Chip8::emulateCycle", "Unknown opcode [0xE000]: Ex" + opcode);
            pc += 2;
            break;
        }
        }
        break;
    }

    case 0xF:
    {
        switch (low)
        {
        case 0x07:
        {
            // Fx07 - LD Vx, DT
            // Set Vx = delay timer value.
            V[X] = delay_timer;
            pc += 2;
            break;
        }

        case 0x0A:
        {
            // Fx0A - LD Vx, K
            // Wait for a key press, store the value of the key in Vx.

            bool keyPressed = false;

            for (uint i = 0; i < 16; i++)
            {
                if (key[i] != 0)
                {
                    V[X] = i;
                    keyPressed = true;
                }
            }

            // Repeat cycle if not pressed
            if (!keyPressed)
                return;

            pc += 2;
            break;
        }

        case 0x15:
        {
            // Fx15 - LD DT, Vx
            // Set delay timer = Vx.
            delay_timer = V[X];
            pc += 2;
            break;
        }

        case 0x18:
        {
            // Fx18 - LD ST, Vx
            // Set sound timer = Vx.
            sound_timer = V[X];
            pc += 2;
            break;
        }

        case 0x1E:
        {
            // Fx1E - ADD I, Vx
            // Set I = I + Vx.
            I += V[X];
            pc += 2;
            break;
        }

        case 0x29:
        {
            // Fx29 - LD F, Vx
            // Set I = location of sprite for digit Vx.
            I = V[X] * 5;
            pc += 2;
            break;
        }

        case 0x33:
        {
            // Fx33 - LD B, Vx
            // Store BCD representation of Vx in memory locations I, I+1, and I+2.
            memory[I] = V[X] / 100;
            memory[I + 1] = (V[X] / 10) % 10;
            memory[I + 2] = (V[X] % 100) % 10;
            pc += 2;
            break;
        }

        case 0x55:
        {
            // Fx55 - LD [I], Vx
            // Store registers V0 through Vx in memory starting at location I.
            // Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.

            // For each register
            for (uint a = 0; a <= X; a++)
            {
                // Save the register's data
                memory[I + a] = V[a];
            }

            // This conflicts with Wikipedia's description but matches BYTE Magazine Vol 3 Num 12 p110
            I += X + 1;

            pc += 2;
            break;
        }

        case 0x65:
        {
            // Fx65 - LD Vx, [I]
            // Read registers V0 through Vx from memory starting at location I.
            // Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.

            // For each register
            for (int a = 0; a <= X; a++)
            {
                // Load memory into the register
                V[a] = memory[I + a];
            }

            // This conflicts with Wikipedia's description but matches BYTE Magazine Vol 3 Num 12 p110
            I += X + 1;

            pc += 2;
            break;
        }

        default:
        {
            // printf("Unknown opcode [0xF000]: Fx%X\n", opcode);
            log(cycle, "ERR", "Chip8::emulateCycle", "Unknown opcode [0xF000]: Fx" + opcode);
            pc += 2;
            break;
        }
        }
        break;
    }

    default:
    {
        printf("Unknown opcode: 0x%X\n", opcode);
        pc += 2;
    }
    }

    // update timers
    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0)
    {
        if (sound_timer == 1)
        {
            // printf("BEEP!\n");
            // log(cycle, "TRC", "Chip8::emulateCycle", "Beep");
        }

        --sound_timer;
    }
}

bool Chip8::loadProgram(const char *file_path)
{
    FILE *program = fopen(file_path, "rb");

    // verify file was loaded
    if (program == NULL)
    {
        log(cycle, "ERR", "Chip8::loadProgram", "Failed to open program");
        // std::cerr << "[loadProgram] Failed to open program" << std::endl;
        return false;
    }

    // get buffer size
    fseek(program, 0, SEEK_END);
    ulong program_size = ftell(program);
    rewind(program);

    // allocate a buffer
    char *program_buffer = (char *)malloc(sizeof(char) * program_size);

    // verify buffer was allocated
    if (program_buffer == NULL)
    {
        // std::cerr << "[loadProgram] Failed to allocate memory for program" << std::endl;
        log(cycle, "ERR", "Chip8::loadProgram", "Failed to allocate memory for program");
        return false;
    }

    // read program into buffer
    size_t result = fread(program_buffer, sizeof(char), (size_t)program_size, program);

    // verify program can be read
    if (result != program_size)
    {
        // std::cerr << "[loadProgram] Failed to read program" << std::endl;
        log(cycle, "ERR", "Chip8::loadProgram", "Failed to read program");
        return false;
    }

    // verify program fits in memory
    if ((4096 - 0x200) < program_size)
    {
        // std::cerr << "[loadProgram] Program too large to fit in memory" << std::endl;
        log(cycle, "ERR", "Chip8::loadProgram", "Program too large to fit in memory");
        return false;
    }

    // transfer the buffer contents into the emulator's memory
    for (uint i = 0; i < program_size; ++i)
    {
        // load into memory starting at 0x200
        memory[i + 0x200] = program_buffer[i];
    }

    fclose(program);
    free(program_buffer);

    return true;
}
