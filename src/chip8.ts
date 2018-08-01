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
  memory: string[];

  /**
   * General purpose 8-bit registers (V0 - VF).
   * Register F should not be used by programs as it is used as a flag by some instructions.
   */
  V: string[];

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

  loadProgram = (program: string) => {
    this.memory.splice(200, program.length, ...program.split(''));
  }

  emulateCycle = () => {
    // do things
    const thing = this.V[0xF]
  }
}

export default Chip8;
