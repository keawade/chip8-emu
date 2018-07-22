#include <stdio.h>
#include <iostream>

#include "chip8.h"

Chip8::Chip8() {}

Chip8::~Chip8() {}

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

void Chip8::initialize()
{
    pc = 0x200; // Program counter starts at 0x200
    opcode = 0; // Reset current opcode
    I = 0;      // Reset index register
    sp = 0;     // Reset stack pointer

    // initialize clear display
    for (int i = 0; i < 2048; ++i)
        gfx[i] = 0;

    // initialize clear stack
    for (int i = 0; i < 16; ++i)
        stack[i] = 0;

    // initialize clear registers V0-VF
    for (int i = 0; i < 16; ++i)
        V[i] = 0;

    // initialize clear keyboard state
    for (int i = 0; i < 16; ++i)
        key[i] = 0;

    // initialize clear memory
    for (int i = 0; i < 4096; ++i)
        memory[i] = 0;

    // load font set
    for (int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];

    // reset timers
    delay_timer = 0;
    sound_timer = 0;
}

void Chip8::emulateCycle()
{
    // fetch opcode
    // Store both bytes of the opcode in a single two byte variable
    opcode = memory[pc] << 8 | memory[pc + 1];

    // decode and execute opcode
    switch (opcode & 0xF000)
    {
    case 0x0000:
    {
        switch (opcode & 0x000F)
        {
        case 0x0000:
        {
            // 00E0 - CLS
            // Clear the display.
            // TODO
            pc += 2;
            break;
        }
        case 0x000E:
        {
            // 00EE - RET
            // Return from a subroutine.
            --sp;
            pc = stack[sp];
            pc += 2;
            break;
        }
        default:
            printf("Unknown opcode [0x0000]: 0x%hd\n", opcode);
            pc += 2;
        }
        break;
    }

    case 0x1000: // 1nnn: JP addr - set program counter to nnn
    {
        pc = opcode & 0x0FFF;
        break;
    }

    case 0x2000:
    {
        // 2nnn - CALL addr
        // Call subroutine at nnn.
        ++sp;
        stack[sp] = pc;
        pc = opcode & 0x0FFF;
        break;
    }

    case 0x3000:
    {
        // 3xkk - SE Vx, byte
        // Skip next instruction if Vx = kk.
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
        {
            pc += 2;
        }
        pc += 2;
        break;
    }
    case 0x4000:
    {
        // 4xkk - SNE Vx, byte
        // Skip next instruction if Vx != kk.
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
        {
            pc += 2;
        }
        pc += 2;
        break;
    }
    case 0x5000:
    {
        // 5xy0 - SE Vx, Vy
        // Skip next instruction if Vx = Vy.
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
        {
            pc += 2;
        }
        pc += 2;
        break;
    }

    case 0x6000:
    {
        // 6xkk - LD Vx, byte
        // Set Vx = kk.
        V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
        pc += 2;
        break;
    }
    case 0x7000:
    {
        // 7xkk - ADD Vx, byte
        // Set Vx = Vx + kk.
        V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
        pc += 2;
        break;
    }
    case 0x8000:
    {
        switch (opcode & 0x000F)
        {
        case 0x0000:
        {
            // 8xy0 - LD Vx, Vy
            // Set Vx = Vy.
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        }
        case 0x0001:
        {
            // 8xy1 - OR Vx, Vy
            // Set Vx = Vx OR Vy.
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        }
        case 0x0002:
        {
            // 8xy2 - AND Vx, Vy
            // Set Vx = Vx AND Vy.
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        }
        case 0x0003:
        {
            // 8xy3 - XOR Vx, Vy
            // Set Vx = Vx XOR Vy.
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        }
        case 0x0004:
        {
            // 8xy4 - ADD Vx, Vy
            // Set Vx = Vx + Vy, set VF = carry.
            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1; // carry
            else
                V[0xF] = 0;

            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        }
        case 0x0005:
        {
            // 8xy5 - SUB Vx, Vy
            // Set Vx = Vx - Vy, set VF = NOT borrow.
            if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
            {
                V[0xF] = 1;
            }
            else
            {
                V[0xF] = 0;
            }
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;
        }
        case 0x0006:
        {
            // 8xy6 - SHR Vx {, Vy}
            // Set Vx = Vx SHR 1.
            // Shifts VY right by one and stores the result to VX (VY remains unchanged). VF is set to the value of the least significant bit of VY before the shift.

            // Store LSD of Vy in VF
            V[0xF] = V[(opcode & 0x00F0) >> 4] & 1;

            // Store Vy >> 1 in Vx
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] >> 1;
            pc += 2;
            break;
        }
        case 0x0007:
        {
            // 8xy7 - SUBN Vx, Vy
            // Set Vx = Vy - Vx, set VF = NOT borrow.
            if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
            {
                V[0xF] = 1;
            }
            else
            {
                V[0xF] = 0;
            }
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        }
        case 0x000E:
        {
            // 8xyE - SHL Vx {, Vy}
            // Set Vy = Vx = Vy SHL 1.
            // Shifts VY left by one and copies the result to VX. VF is set to the value of the most significant bit of VY before the shift.

            // Store MSD of Vy in VF
            V[0xF] = V[(opcode & 0x00F0) >> 4] >> 7;

            // Store Vy << 1 in Vx
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] << 1;

            // Store Vy << 1 in Vy
            V[(opcode & 0x00F0) >> 4] = V[(opcode & 0x00F0) >> 4] << 1;
            pc += 2;
            break;
        }
        default:
        {
            printf("Unknown opcode [0x8000]: 8x%hd\n", opcode);
            pc += 2;
            break;
        }
        }
        break;
    }

    case 0x9000:
    {
        // 9xy0 - SNE Vx, Vy
        // Skip next instruction if Vx != Vy.
        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
        {
            pc += 2;
        }
        pc += 2;
        break;
    }
    case 0xA000:
    {
        // Annn - LD I, addr
        // Set I = nnn.
        I = opcode & 0x0FFF;
        pc += 2;
        break;
    }
    case 0xB000:
    {
        // Bnnn - JP V0, addr
        // Jump to location nnn + V0.
        pc = (opcode & 0x0FFF) + V[0];
        break;
    }
    case 0xC000:
    {
        // Cxkk - RND Vx, byte
        // Set Vx = random byte AND kk.
        V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
        pc += 2;
        break;
    }
    case 0xD000:
    {
        // Dxyn - DRW Vx, Vy, nibble
        // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        int8_t x = V[(opcode & 0x0F00) >> 8];
        int8_t y = V[(opcode & 0x00F0) >> 4];
        int8_t height = opcode & 0x000F;
        int8_t pixel;

        // Reset VF. We'll set this to 1 later if there is a collision.
        V[0xF] = 0;

        // For every line on the y axis
        for (int yline = 0; yline < height; yline++)
        {
            // Get the pixel data from memory
            pixel = memory[I + yline];
            for (int xline = 0; xline < 8; xline++)
            {
                // 0x80 == 0b10000000
                // For each x value, check if it is to be toggled (pixel AND current value from mem != 0)
                if ((pixel & (0x80 >> xline)) != 0)
                {
                    // If a pixel is to be toggled
                    // Check if if the value is already on
                    if (gfx[(x + xline + ((y + yline) * 64))] == 1)
                    {
                        // If it is already on, set VF to 1
                        V[0xF] = 1;
                    }
                    // XOR the given value with the new value
                    gfx[x + xline + ((y + yline) * 64)] ^= 1;
                }
            }
        }

        // Set the drawFlag to tell the system to update the screen
        drawFlag = true;

        pc += 2;
        break;
    }

    case 0xE000:
    {
        switch (opcode & 0x00FF)
        {
        case 0x009E:
        {
            // Ex9E - SKP Vx
            // Skip next instruction if key with the value of Vx is pressed.
            if (key[V[(opcode & 0x0F00) >> 8]])
            {
                pc += 2;
            }
            pc += 2;
            break;
        }
        case 0x00A1:
        {
            // ExA1 - SKNP Vx
            // Skip next instruction if key with the value of Vx is not pressed.
            if (!key[V[(opcode & 0x0F00) >> 8]])
            {
                pc += 2;
            }
            pc += 2;
            break;
        }
        default:
        {
            printf("Unknown opcode [0xE000]: Ex%hd\n", opcode);
            pc += 2;
            break;
        }
        }
        break;
    }

    case 0xF000:
    {
        switch (opcode & 0x00FF)
        {
        case 0x0007:
        {
            // Fx07 - LD Vx, DT
            // Set Vx = delay timer value.
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            pc += 2;
            break;
        }

        case 0x000A:
        {
            // Fx0A - LD Vx, K
            // Wait for a key press, store the value of the key in Vx.
            pc += 2;
            break;
        }

        case 0x0015:
        {
            // Fx15 - LD DT, Vx
            // Set delay timer = Vx.
            delay_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        }

        case 0x0018:
        {
            // Fx18 - LD ST, Vx
            // Set sound timer = Vx.
            sound_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        }

        case 0x001E:
        {
            // Fx1E - ADD I, Vx
            // Set I = I + Vx.
            I += V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;
        }

        case 0x0029:
        {
            // Fx29 - LD F, Vx
            // Set I = location of sprite for digit Vx.
            I = memory[((opcode & 0x0F00) >> 8) * 5];
            pc += 2;
            break;
        }

        case 0x0033:
        {
            // Fx33 - LD B, Vx
            // Store BCD representation of Vx in memory locations I, I+1, and I+2.
            memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            pc += 2;
            break;
        }

        case 0x0055:
        {
            // Fx55 - LD [I], Vx
            // Store registers V0 through Vx in memory starting at location I.
            int8_t x = (opcode & 0x0F00) >> 8;
            // For each register
            for (int a = 0; a < x; a++)
            {
                // Save the register's data
                memory[I + a] = V[a];
            }
            pc += 2;
            break;
        }

        case 0x0065:
        {
            // Fx65 - LD Vx, [I]
            // Read registers V0 through Vx from memory starting at location I.
            int8_t x = (opcode & 0x0F00) >> 8;
            // For each register
            for (int a = 0; a < x; a++)
            {
                // Load memory into the register
                V[a] = memory[I + a];
            }
            pc += 2;
            break;
        }

        default:
        {
            printf("Unknown opcode [0xF000]: Fx%hd\n", opcode);
            pc += 2;
            break;
        }
        }
        break;
    }

    default:
    {
        printf("Unknown opcode: 0x%hd\n", opcode);
        pc += 2;
    }
    }

    // update timers
    if (delay_timer > 0)
        --delay_timer;

    if (sound_timer > 0)
    {
        if (sound_timer == 1)
            printf("BEEP!\n");

        --sound_timer;
    }
}

bool Chip8::loadProgram(const char *file_path)
{
    FILE *program = fopen(file_path, "rb");

    // verify file was loaded
    if (program == NULL)
    {
        std::cerr << "[loadProgram] Failed to open program" << std::endl;
        return false;
    }

    // get buffer size
    fseek(program, 0, SEEK_END);
    unsigned long program_size = ftell(program);
    rewind(program);

    // allocate a buffer
    char *program_buffer = (char *)malloc(sizeof(char) * program_size);

    // verify buffer was allocated
    if (program_buffer == NULL)
    {
        std::cerr << "[loadProgram] Failed to allocate memory for program" << std::endl;
        return false;
    }

    // read program into buffer
    size_t result = fread(program_buffer, sizeof(char), (size_t)program_size, program);

    // verify program can be read
    if (result != program_size)
    {
        std::cerr << "[loadProgram] Failed to read program" << std::endl;
        return false;
    }

    // verify program fits in memory
    if ((4096 - 0x200) < program_size)
    {
        std::cerr << "[loadProgram] Program too large to fit in memory" << std::endl;
        return false;
    }

    // transfer the buffer contents into the emulator's memory
    for (unsigned int i = 0; i < program_size; ++i)
    {
        // load into memory starting at 0x200
        memory[i + 0x200] = program_buffer[i];
    }

    fclose(program);
    free(program_buffer);

    return true;
}
