import * as _ from 'lodash';
import { Prog } from './programs';

/**
 * Chip-8 Emulator
 */
export class Chip8 {
  /**
   * Cycle counter for debugging.
   */
  private cycle: number;

  /**
   * 4096 bytes of memory.
   * Handles all storage other than the stack and registers.
   */
  private memory: number[];

  /**
   * General purpose 8-bit registers (V0 - VF).
   * Register F should not be used by programs as it is used as a flag by some instructions.
   */
  private V: number[];

  /**
   * 16-bit register I.
   * This register is generally used to store memory addresses, so only the lowest (rightmost) 12 bits are usually used.
   */
  private I: number;

  /**
   * 16-bit program counter.
   * Stores the currently executing address.
   */
  private pc: number;

  /**
   * 8-bit delay timer.
   * This timer does nothing more than subtract 1 from the value of DT at a rate of 60Hz. When DT reaches 0, it deactivates.
   */
  private delay_timer: number;

  /**
   * 8-bit sound timer.
   * This timer also decrements at a rate of 60Hz, however, as long as sound_timer's value is greater than zero, the Chip-8 buzzer will sound. When sound_timer reaches zero, the sound timer deactivates.
   */
  private sound_timer: number;

  /**
   * Array of sixteen 16-bit values.
   * Used to store the address that the interpreter shoud return to when finished with a subroutine
   */

  private stack: number[];

  /**

   * 8-bit stack pointer.
   * Points to the topmost level of the stack.
   */
  private sp: number;

  /**
   * Array storing the current screen state.
   */
  public graphics: boolean[][];

  /**
   * Flag to trigger drawing.
   */
  public drawFlag: boolean;

  /**
   * Array of keyboard states.
   */
  public keyboard: boolean[];

  constructor() {
    this.cycle = 0;

    this.pc = 0x200;
    this.I = 0;
    this.sp = 0;

    // Clear stack
    this.stack = _.fill(Array(16), 0);

    // Clear display
    this.graphics = _.fill(Array(32), _.fill(Array(64), false));
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

  /**
   * Loads the provided program.
   * @param {Prog} program - An object containing the program name and the program binary in hex.
   */
  public loadProgram = (program: Prog) => {
    console.log('[Chip8] loading program', program.name);

    this.memory.splice(0x200, program.code.length, ...program.code.split('').map(n => parseInt(n, 16)));
  }

  /**
   * Sets a key state to "On".
   * @param {string} key - The key to set.
   * @param {boolean} state - The state of the key
   */
  public setKey = (key: string, state: boolean) => {
    const keys = ['1', '2', '3', 'C', '4', '5', '6', 'D', '7', '8', '9', 'E', 'A', '0', 'B', 'F'];
    const toPress = keys.sort().indexOf(key.toLowerCase());
    this.keyboard[toPress] = state;
  }

  /**
   * Sets all key states to "Off".
   */
  public clearKeys = () => {
    this.keyboard = _.fill(Array(16), false);
  }

  private uknownOpcode = (opcode: number) => {
    console.warn('[Chip8] emulateCycle - Unknown opcode: 0x' + opcode.toString(16));
    this.pc += 2;
  }

  /**
   * Reads and executes the next opcode.
   */
  public emulateCycle = () => {
    this.cycle += 1;

    /**
     * 16-bit operation code.
     * Stores the current opcode.
     */
    const opcode = this.memory[this.pc] << 8 | this.memory[this.pc + 1];

    switch (opcode & 0xF000) {
      case 0x0000:
        switch (opcode & 0x00FF) {
          case 0x00E0:
            // 00E0 - CLS
            // Clear the display.

            this.graphics = _.fill(Array(64), _.fill(Array(32), false));
            this.drawFlag = true;

            this.pc += 2;
            break;
          case 0x00EE:
            // 00EE - RET
            // Return from a subroutine.

            this.sp -= 1;
            this.pc = this.stack[this.sp];

            this.pc += 2;
            break;
          default:
            this.uknownOpcode(opcode);
            break;
        }
        break;

      case 0x1000:
        // 1nnn: JP addr - set program counter to nnn

        this.pc = opcode & 0x0FFF;
        break;

      case 0x2000:
        // 2nnn - CALL addr
        // Call subroutine at nnn.

        this.stack[this.sp] = this.pc;
        this.sp += 1;
        this.pc = opcode & 0x0FFF;
        break;

      case 0x3000:
        // 3xkk - SE Vx, byte
        // Skip next instruction if Vx = kk.

        if (this.V[(opcode & 0x0F00) >>> 8] == (opcode & 0x00FF)) {
          this.pc += 2;
        }

        this.pc += 2;
        break;

      case 0x4000:
        // 4xkk - SNE Vx, byte
        // Skip next instruction if Vx != kk.

        if (this.V[(opcode & 0x0F00) >>> 8] != (opcode & 0x00FF)) {
          this.pc += 2;
        }

        this.pc += 2;
        break;

      case 0x5000:
        // 5xy0 - SE Vx, Vy
        // Skip next instruction if Vx = Vy.

        if (this.V[(opcode & 0x0F00) >>> 8] == this.V[(opcode & 0x00F0) >>> 4]) {
          this.pc += 2;
        }

        this.pc += 2;
        break;

      case 0x6000:
        // 6xkk - LD Vx, byte
        // Set Vx = kk.

        this.V[(opcode & 0x0F00) >>> 8] = opcode & 0x00FF;

        this.pc += 2;
        break;

      case 0x7000:
        // 7xkk - ADD Vx, byte
        // Set Vx = Vx + kk.

        this.V[(opcode & 0x0F00) >>> 8] += opcode & 0x00FF;

        this.pc += 2;
        break;

      case 0x8000:
        switch (opcode & 0x000F) {
          case 0x0000:
            // 8xy0 - LD Vx, Vythis.V[(opcode & 0x00F0) >>> 4]
            // Set Vx = Vy.

            this.V[(opcode & 0x0F00) >>> 8] = this.V[(opcode & 0x00F0) >>> 4];

            this.pc += 2;
            break;

          case 0x0001:
            // 8xy1 - OR Vx, Vy
            // Set Vx = Vx OR Vy.

            this.V[(opcode & 0x0F00) >>> 8] = this.V[(opcode & 0x0F00) >>> 8] | this.V[(opcode & 0x00F0) >>> 4];

            this.pc += 2;
            break;

          case 0x0002:
            // 8xy2 - AND Vx, Vy
            // Set Vx = Vx AND Vy.

            this.V[(opcode & 0x0F00) >>> 8] = this.V[(opcode & 0x0F00) >>> 8] & this.V[(opcode & 0x00F0) >>> 4];

            this.pc += 2;
            break;

          case 0x0003:
            // 8xy3 - XOR Vx, Vy
            // Set Vx = Vx XOR Vy.

            this.V[(opcode & 0x0F00) >>> 8] = this.V[(opcode & 0x0F00) >>> 8] ^ this.V[(opcode & 0x00F0) >>> 4];

            this.pc += 2;
            break;

          case 0x0004:
            // 8xy4 - ADD Vx, Vy
            // Set Vx = Vx + Vy, set VF = carry.

            this.V[(opcode & 0x0F00) >>> 8] += this.V[(opcode & 0x00F0) >>> 4];

            if (this.V[(opcode & 0x0F00) >>> 8] > this.V[(opcode & 0x00F0) >>> 4])
              this.V[0xF] = 1; // carry
            else
              this.V[0xF] = 0;

            this.pc += 2;
            break;

          case 0x0005:
            // 8xy5 - SUB Vx, Vy
            // Set Vx = Vx - Vy, set VF = NOT borrow.

            if (this.V[(opcode & 0x0F00) >>> 8] > this.V[(opcode & 0x00F0) >>> 4])
              this.V[0xF] = 1;
            else
              this.V[0xF] = 0;

            this.V[(opcode & 0x0F00) >>> 8] -= this.V[(opcode & 0x00F0) >>> 4];

            this.pc += 2;
            break;

          case 0x0006:
            // 8xy6 - SHR Vx {, Vy}
            // Set Vx = Vx SHR 1.
            // Shifts VY right by one and stores the result to VX (VY remains unchanged). VF is set to the value of the least significant bit of VY before the shift.

            this.V[0xF] = this.V[(opcode & 0x0F00) >>> 8] & 1;
            this.V[(opcode & 0x0F00) >>> 8] /= 2;

            this.pc += 2;
            break;

          case 0x0007:
            // 8xy7 - SUBN Vx, Vy
            // Set Vx = Vy - Vx, set VF = NOT borrow.

            if (this.V[(opcode & 0x0F00) >>> 8] > this.V[(opcode & 0x00F0) >>> 4])
              this.V[0xF] = 1;
            else
              this.V[0xF] = 0;

            this.V[(opcode & 0x0F00) >>> 8] = this.V[(opcode & 0x00F0) >>> 4] - this.V[(opcode & 0x0F00) >>> 8];

            this.pc += 2;
            break;

          case 0x000E:
            // 8xyE - SHL Vx {, Vy}
            // Set Vy = Vx = Vy SHL 1.
            // Shifts VY left by one and copies the result to VX. VF is set to the value of the most significant bit of VY before the shift.

            this.V[0xF] = this.V[(opcode & 0x00F0) >>> 4] >> 7;

            this.V[(opcode & 0x0F00) >>> 8] = this.V[(opcode & 0x0F00) >>> 8] * 2;

            this.pc += 2;
            break;

          default:
            this.uknownOpcode(opcode);
            break;
        }
        break;

      case 0x9000:
        // 9xy0 - SNE Vx, Vy
        // Skip next instruction if Vx != Vy.

        if (this.V[(opcode & 0x0F00) >>> 8] != this.V[(opcode & 0x00F0) >>> 4]) {
          this.pc += 2;
        }

        this.pc += 2;
        break;

      case 0xA000:
        // Annn - LD I, addr
        // Set I = nnn.

        this.I = opcode & 0x0FFF;

        this.pc += 2;
        break;

      case 0xB000:
        // Bnnn - JP V0, addr
        // Jump to location nnn + V0.

        this.pc = (opcode & 0x0FFF) + this.V[0x0];

        break;

      case 0xC000:
        // Cxkk - RND Vx, byte
        // Set Vx = random byte AND kk.

        this.V[(opcode & 0x0F00) >>> 8] = (Math.floor(Math.random() * 0xFF)) & (opcode & 0x00FF);

        this.pc += 2;
        break;

      case 0xD000:
        // Dxyn - DRW Vx, Vy, nibble
        // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        let pixel;

        // Reset VF. We'll set this to 1 later if there is a collision.
        this.V[0xF] = 0;

        // For every line on the y axis
        for (let yline = 0; yline < (opcode & 0x000F); yline++) {
          // Get the pixel data from memory
          pixel = this.memory[this.I + yline];
          for (let xline = 0; xline < 8; xline++) {
            // 0x80 == 0b10000000
            // For each x value, check if it is to be toggled (pixel AND current value from mem != 0)
            if ((pixel & (0x80 >> xline)) !== 0) {
              // If a pixel is to be toggled
              // Check if if the value is already on
              if (this.graphics[this.V[(opcode & 0x0F00) >>> 8] + xline][this.V[(opcode & 0x00F0) >>> 4] + yline] === true) {
                // if (gfx[(V[X] + xline + ((V[Y] + yline) * 64))] == 1)
                // If it is already on, set VF to 1
                this.V[0xF] = 1;
              }
              // XOR the given value with the new value
              // gfx[V[X] + xline + ((V[Y] + yline) * 64)] ^= 1;
              this.graphics[this.V[(opcode & 0x0F00) >>> 8] + xline][this.V[(opcode & 0x00F0) >>> 4] + yline] !== true; // TODO: Verify
            }
          }
        }

        // Set the drawFlag to tell the system to update the screen
        this.drawFlag = true;

        this.pc += 2;
        break;

      case 0xE000:
        switch (opcode & 0x00FF) {
          case 0x009E:
            // Ex9E - SKP Vx
            // Skip next instruction if key with the value of Vx is pressed.

            if (this.keyboard[this.V[(opcode & 0x0F00) >>> 8]] === true) {
              this.pc += 2;
            }

            this.pc += 2;
            break;

          case 0x00A1:
            // ExA1 - SKNP Vx
            // Skip next instruction if key with the value of Vx is not pressed.

            if (this.keyboard[this.V[(opcode & 0x0F00) >>> 8]] === false) {
              this.pc += 2;
            }

            this.pc += 2;
            break;

          default:
            this.uknownOpcode(opcode);
            break;
        }
        break;

      case 0xF000:
        switch (opcode & 0x00FF) {
          case 0x0007:
            // Fx07 - LD Vx, DT
            // Set Vx = delay timer value.

            this.V[(opcode & 0x0F00) >>> 8] = this.delay_timer;

            this.pc += 2;
            break;

          case 0x000A:
            // Fx0A - LD Vx, K
            // Wait for a key press, store the value of the key in Vx.

            let keyPressed = false;

            for (let i = 0; i < 16; i++) {
              if (this.keyboard[i] === true) {
                this.V[(opcode & 0x0F00) >>> 8] = i;
                keyPressed = true;
              }
            }

            // Repeat cycle if not pressed
            if (!keyPressed)
              return;

            this.pc += 2;
            break;

          case 0x0015:
            // Fx15 - LD DT, Vx
            // Set delay timer = Vx.

            this.delay_timer = this.V[(opcode & 0x0F00) >>> 8];

            this.pc += 2;
            break;

          case 0x0018:
            // Fx18 - LD ST, Vx
            // Set sound timer = Vx.

            this.sound_timer = this.V[(opcode & 0x0F00) >>> 8];

            this.pc += 2;
            break;

          case 0x001E:
            // Fx1E - ADD I, Vx
            // Set I = I + Vx.

            this.I += this.V[(opcode & 0x0F00) >>> 8];

            this.pc += 2;
            break;

          case 0x0029:
            // Fx29 - LD F, Vx
            // Set I = location of sprite for digit Vx.
            this.I = this.V[(opcode & 0x0F00) >>> 8] * 5;
            this.pc += 2;
            break;

          case 0x0033:
            // Fx33 - LD B, Vx
            // Store BCD representation of Vx in memory locations I, I+1, and I+2.
            this.memory[this.I] = this.V[(opcode & 0x0F00) >>> 8] / 100;
            this.memory[this.I + 1] = (this.V[(opcode & 0x0F00) >>> 8] / 10) % 10;
            this.memory[this.I + 2] = (this.V[(opcode & 0x0F00) >>> 8] % 100) % 10;
            this.pc += 2;
            break;

          case 0x0055:
            // Fx55 - LD [I], Vx
            // Store registers V0 through Vx in memory starting at location I.
            // Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.

            // For each register
            for (let a = 0; a <= ((opcode & 0x0F00) >>> 8); a++) {
              // Save the register's data
              this.memory[this.I + a] = this.V[a];
            }

            // This conflicts with Wikipedia's description but matches BYTE Magazine Vol 3 Num 12 p110
            this.I += ((opcode & 0x0F00) >>> 8) + 1;

            this.pc += 2;
            break;

          case 0x0065:
            // Fx65 - LD Vx, [I]
            // Read registers V0 through Vx from memory starting at location I.
            // Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.

            // For each register
            for (let a = 0; a <= ((opcode & 0x0F00) >>> 8); a++) {
              // Load memory into the register
              this.V[a] = this.memory[this.I + a];
            }

            // This conflicts with Wikipedia's description but matches BYTE Magazine Vol 3 Num 12 p110
            this.I += ((opcode & 0x0F00) >>> 8) + 1;

            this.pc += 2;
            break;

          default:
            this.uknownOpcode(opcode);
            break;
        }
        break;

      default:
        this.uknownOpcode(opcode);
        break;
    }
  }
}

/**
 * 4*5 pixel arrangements for the characters 0 - F
 */
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
