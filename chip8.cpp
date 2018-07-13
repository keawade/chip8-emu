#include <stdio.h>
#include <iostream>

#include "chip8.h"

Chip8::Chip8() {}
Chip8::~Chip8() {}

int8_t chip8_fontset[80] =
    {
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

  // clear display
  // clear stack
  // clear registers V0-VF
  // clear memory

  // load fontset
  for (int i = 0; i < 80; ++i)
    memory[i] = chip8_fontset[i];

  // reset timers
}

bool Chip8::load(const char *file_path)
{
  FILE *program = fopen(file_path, "rb");

  // verify file was loaded
  if (program == NULL)
  {
    std::cerr << "Failed to open program" << std::endl;
    return false;
  }

  // get buffer size
  fseek(program, 0, SEEK_END);
  long program_size = ftell(program);
  rewind(program);

  // allocate a buffer
  char *program_buffer = (char *)malloc(sizeof(char) * program_size);

  // verify buffer was allocated
  if (program_buffer == NULL)
  {
    std::cerr << "Failed to allocate memory for program" << std::endl;
    return false;
  }

  // read program into buffer
  size_t result = fread(program_buffer, sizeof(char), (size_t)program_size, program);

  // verify program can be read
  if (result != program_size)
  {
    std::cerr << "Failed to read program" << std::endl;
    return false;
  }

  // verify program fits in memory
  if ((4096 - 0x200) > program_size)
  {
    std::cerr << "Program too large to fit in memory" << std::endl;
    return false;
  }

  // transfer the buffer contents into the emulator's memory
  for (int i = 0; i < program_size; ++i)
  {
    // load into memory starting at 0x200
    memory[i + 0x200] = program_buffer[i];
  }

  fclose(program);
  free(program_buffer);

  return true;
}

void Chip8::emulateCycle()
{
  // fetch opcode
  opcode = memory[pc] << 8 | memory[pc + 1];

  // decode and execute opcode
  switch (opcode & 0xF000)
  {
  case 0x0000:
    switch (opcode & 0x000F)
    {
    case 0x0000:
      // 00E0 - CLS
      // Clear the display.
      break;

    case 0x000E:
      // 00EE - RET
      // Return from a subroutine.
      break;

    default:
      printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
    }
    break;

  case 0x1000: // 1nnn: JP addr - set program counter to nnn
    pc = opcode & 0x0FFF;
    break;

  case 0x2000:
    // 2nnn - CALL addr
    // Call subroutine at nnn.
    I = opcode & 0x0FFF;
    pc += 2;
    break;

    // 3xkk - SE Vx, byte
    // Skip next instruction if Vx = kk.

    // 4xkk - SNE Vx, byte
    // Skip next instruction if Vx != kk.

    // 5xy0 - SE Vx, Vy
    // Skip next instruction if Vx = Vy.

    // 6xkk - LD Vx, byte
    // Set Vx = kk.

    // 7xkk - ADD Vx, byte
    // Set Vx = Vx + kk.

    // 8xy0 - LD Vx, Vy
    // Set Vx = Vy.

    // 8xy1 - OR Vx, Vy
    // Set Vx = Vx OR Vy.

    // 8xy2 - AND Vx, Vy
    // Set Vx = Vx AND Vy.

    // 8xy3 - XOR Vx, Vy
    // Set Vx = Vx XOR Vy.

  case 0x0004:
    // 8xy4 - ADD Vx, Vy
    // Set Vx = Vx + Vy, set VF = carry.
    if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
      V[0xF] = 1; // carry
    else
      V[0xF] = 0;

    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
    pc += 2;
    break;

    // 8xy5 - SUB Vx, Vy
    // Set Vx = Vx - Vy, set VF = NOT borrow.

    // 8xy6 - SHR Vx {, Vy}
    // Set Vx = Vx SHR 1.

    // 8xy7 - SUBN Vx, Vy
    // Set Vx = Vy - Vx, set VF = NOT borrow.

    // 8xyE - SHL Vx {, Vy}
    // Set Vx = Vx SHL 1.

    // 9xy0 - SNE Vx, Vy
    // Skip next instruction if Vx != Vy.

    // Annn - LD I, addr
    // Set I = nnn.

    // Bnnn - JP V0, addr
    // Jump to location nnn + V0.

    // Cxkk - RND Vx, byte
    // Set Vx = random byte AND kk.

    // Dxyn - DRW Vx, Vy, nibble
    // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

    // Ex9E - SKP Vx
    // Skip next instruction if key with the value of Vx is pressed.

    // ExA1 - SKNP Vx
    // Skip next instruction if key with the value of Vx is not pressed.

    // Fx07 - LD Vx, DT
    // Set Vx = delay timer value.

    // Fx0A - LD Vx, K
    // Wait for a key press, store the value of the key in Vx.

    // Fx15 - LD DT, Vx
    // Set delay timer = Vx.

    // Fx18 - LD ST, Vx
    // Set sound timer = Vx.

    // Fx1E - ADD I, Vx
    // Set I = I + Vx.

    // Fx29 - LD F, Vx
    // Set I = location of sprite for digit Vx.

  case 0x0033:
    // Fx33 - LD B, Vx
    // Store BCD representation of Vx in memory locations I, I+1, and I+2.
    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
    pc += 2;
    break;

    // Fx55 - LD [I], Vx
    // Store registers V0 through Vx in memory starting at location I.

    // Fx65 - LD Vx, [I]
    // Read registers V0 through Vx from memory starting at location I.

  default:
    printf("Unknown opcode: 0x%X\n", opcode);
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