#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdint.h>

class Chip8
{
public:
  int cycle; // Debugging tool

  // u_int16_t opcode;
  u_int8_t memory[4096];

  // General purpose 8-bit registers (V0 - VF)
  // register F should not be used by programs as it is used as a flag by some instructions.
  u_int8_t V[16];

  // This register is generally used to store memory addresses, so only the lowest (rightmost) 12 bits are usually used.
  u_int16_t I;

  // Program Counter (pc)
  // Stores the currently executing address
  u_int16_t pc;

  // 8-bit delay timer
  // This timer does nothing more than subtract 1 from the value of DT at a rate of 60Hz. When DT reaches 0, it deactivates.
  u_int8_t delay_timer;

  // 8-bit sound timer
  // This timer also decrements at a rate of 60Hz, however, as long as ST's value is greater than zero, the Chip-8 buzzer will sound. When ST reaches zero, the sound timer deactivates.
  u_int8_t sound_timer;

  // Stack
  // Array of 16 16-bit values, used to store the address that theu_interpreter shoud return to when finished with a subroutine
  u_int16_t stack[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // Points to the topmost level of the stack.
  u_int8_t sp = 0x0;

  void initialize();

  // Graphics
  u_int8_t gfx[64 * 32];
  bool drawFlag;

  // Keyboard state
  bool key[16];

  union { // Provide easier interfaces for opcodes
    std::uint16_t opcode;

    struct {
      uint N : 4;
      uint Y : 4;
      uint X : 4;
      uint A : 4;
    };

    struct {
      ushort low : 8;
      ushort high : 8;
    };

    struct {
      ushort NNN : 12;
      uint NA : 4; 
    };
  };

  Chip8();
  ~Chip8();

  void emulateCycle();
  bool loadProgram(const char *file_path);
};

#endif // CHIP_8_H
