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
  // decode opcode
  // execute opcode

  // update timers
}