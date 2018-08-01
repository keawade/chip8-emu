import * as _ from 'lodash';

/**
 * Chip-8 Emulator
 */
class Chip8 {
  /**
   * 16-bit operation code.
   * Stores the current opcode.
   */
  opcode: number;

  /**
   * 4096 bytes of memory.
   * Handles all storage other than the stack and registers.
   */
  memory: number[];

  /**
   * General purpose 8-bit registers (V0 - VF).
   * Register F should not be used by programs as it is used as a flag by some instructions.
   */
  V: number[];

  /**
   * 16-bit register I.
   * This register is generally used to store memory addresses, so only the lowest (rightmost) 12 bits are usually used.
   */
  I: number;

  /**
   * 16-bit program counter.
   * Stores the currently executing address.
   */
  pc: number;

  /**
   * 8-bit delay timer.
   * This timer does nothing more than subtract 1 from the value of DT at a rate of 60Hz. When DT reaches 0, it deactivates.
   */
  delay_timer: number;

  /**
   * 8-bit sound timer.
   * This timer also decrements at a rate of 60Hz, however, as long as sound_timer's value is greater than zero, the Chip-8 buzzer will sound. When sound_timer reaches zero, the sound timer deactivates.
   */
  sound_timer: number;

  /**
   * Array of sixteen 16-bit values.
   * Used to store the address that the interpreter shoud return to when finished with a subroutine
   */

  stack: number[];

  /**

   * 8-bit stack pointer.
   * Points to the topmost level of the stack.
   */
  sp: number;

  /**
   * Array storing the current screen state.
   */
  graphics: boolean[][];

  /**
   * Flag to trigger drawing.
   */
  drawFlag: boolean;

  /**
   * Array of keyboard states.
   */
  keyboard: boolean[];

  /**
   * Initializes/resets all the Chip-8 memory, stack, registers, screen, timers, and key states.
   */
  initialize = () => {
    this.pc = 0x200;
    this.opcode = 0;
    this.I = 0;
    this.sp = 0;

    // Clear stack
    this.stack = _.fill(Array(16), 0);

    // Clear display
    this.graphics = _.fill(Array(64), _.fill(Array(32), false));
    this.drawFlag = false;

    // Clear Registers
    this.V = _.fill(Array(16), 0);
    this.I = 0;

    // Clear keyboard state
    this.keyboard = _.fill(Array(16), false);

    // Clear memory
    this.memory = _.fill(Array(0x1000), 0);

    // Load font set
    this.memory.splice(0, FONTSET.length, ...FONTSET);

    // Clear timers
    this.delay_timer = 0;
    this.sound_timer = 0;
  }

  loadProgram = (program: string) => {
    this.memory.splice(0x200, program.length, ...program.split('').map(n => parseInt(n, 16)));
  }

  emulateCycle = () => {
    // do things
    const thing = this.V[0xF]
  }
}

const FONTSET = [
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
];

export default Chip8;
