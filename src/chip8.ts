import * as _ from 'lodash';
import { leftPad } from './leftpad';

enum LogLevels {
  ERROR = 0,
  WARN,
  LOG,
  DEBUG,
}

/**
 * Chip-8 Emulator
 */
export class Chip8 {
  /**
   * 4096 bytes of memory.
   * Handles all storage other than the stack and registers.
   */
  private memory: Uint8Array;

  /**
   * General purpose 8-bit registers (V0 - VF).
   * Register F should not be used by programs as it is used as a flag by some instructions.
   */
  private V: Uint8Array;

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

  /**
   * Array storing the current screen state.
   */
  public graphics: number[];

  /**
   * Flag to trigger drawing.
   */
  public drawFlag: boolean;

  /**
   * Array of keyboard states.
   */
  public keyboard: boolean[];

  /**
   * Current Loglevel.
   * ERROR, WARN, DEBUG, ALL
   */
  public loglevel: LogLevels;

  constructor(loglevel?: LogLevels) {
    this.loglevel = !!loglevel ? loglevel : LogLevels.ERROR;

    this.log(LogLevels.DEBUG, '[Chip8] intializing');

    this.pc = 0x200;
    this.I = 0;

    // Clear stack
    this.stack = [];

    // Clear display
    this.graphics = _.fill(Array(32 * 64), 0);
    this.drawFlag = false;

    // Clear Registers
    this.V = new Uint8Array(16);
    this.I = 0;

    // Clear keyboard state
    this.keyboard = _.fill(Array(16), false);

    // Clear memory
    this.memory = new Uint8Array(new ArrayBuffer(0x1000));

    // Load font set
    for (let i = 0; i < FONTSET.length; i++) {
      this.memory[i] = FONTSET[i];
    }

    // Clear timers
    this.delay_timer = 0;
    this.sound_timer = 0;
  }

  /**
   * Loads the provided program.
   * @param {Uint8Array} program - Program code as an unsigned 8-bit integer array.
   */
  public loadProgram = (program: Uint8Array) => {
    this.log(LogLevels.DEBUG, 'Loading program');

    for (let i = 0; i < program.length; i++) {
      this.memory[i + 0x200] = program[i];
    }
  }

  /**
   * Sets a key state to "On".
   * @param {string} key - The key to set.
   * @param {boolean} state - The state of the key
   */
  public setKey = (key: string, state: boolean) => {
    this.log(LogLevels.DEBUG, '[Chip8] setting key ' + key + ' to state ' + state);

    const keys = ['1', '2', '3', 'C', '4', '5', '6', 'D', '7', '8', '9', 'E', 'A', '0', 'B', 'F'];
    const toPress = keys.sort().indexOf(key.toLowerCase());
    this.keyboard[toPress] = state;
  }

  /**
   * Sets all key states to "Off".
   */
  public clearKeys = () => {
    this.log(LogLevels.DEBUG, 'Clearing keys');

    this.keyboard = _.fill(Array(16), false);
  }

  private log = (level: LogLevels, message: string) => {
    if (level <= this.loglevel) {
      switch (level) {
        case LogLevels.ERROR:
          console.error(message);
          break;
        case LogLevels.WARN:
          console.warn(message);
          break;
        case LogLevels.LOG:
          console.log(message);
          break;
        case LogLevels.DEBUG:
          console.debug(message);
          break;
      }
    }
  }

  private setPixel = (x: number, y: number): boolean => {
    const width = 64;
    const height = 32;

    if (x > width) {
      x -= width;
    } else if (x < 0) {
      x += width;
    }

    if (y > height) {
      y -= height;
    } else if (y < 0) {
      y += height;
    }

    const location = x + (y * width);

    this.graphics[location] ^= 1;

    return !this.graphics[location];
  }

  /**
   * Reads and executes the next opcode.
   */
  public emulateCycle = () => {
    /**
     * 16-bit operation code.
     * Stores the current opcode.
     */
    const opcode = this.memory[this.pc] << 8 | this.memory[this.pc + 1];

    const x = (opcode & 0x0F00) >> 8;
    const y = (opcode & 0x00F0) >> 4;
    const kk = opcode & 0x00FF;
    const nnn = opcode & 0x0FFF;

    switch (opcode & 0xF000) {
      case 0x0000:
        switch (opcode & 0x00FF) {
          case 0x00E0:
            // 00E0 - CLS
            // Clear the display.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - CLS');

            this.graphics = _.fill(Array(32 * 64), 0);
            this.drawFlag = true;

            this.pc += 2;
            break;

          case 0x00EE:
            // 00EE - RET
            // Return from a subroutine.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - RET');

            this.pc = this.stack.pop() as number;

            this.pc += 2;
            break;

          default:
            this.log(LogLevels.ERROR, '[Chip8] emulateCycle - Unknown opcode: 0x' + leftPad(opcode.toString(16), 4, 0));
            this.pc += 2;
            break;
        }
        break;

      case 0x1000:
        // 1nnn: JP addr - set program counter to nnn
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - JP');

        this.pc = nnn;
        break;

      case 0x2000:
        // 2nnn - CALL addr
        // Call subroutine at nnn.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - CALL');

        this.stack.push(this.pc);
        this.pc = nnn;
        break;

      case 0x3000:
        // 3xkk - SE Vx, byte
        // Skip next instruction if Vx = kk.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - SE Vx, byte');

        if (this.V[x] === kk) {
          this.pc += 2;
        }

        this.pc += 2;
        break;

      case 0x4000:
        // 4xkk - SNE Vx, byte
        // Skip next instruction if Vx != kk.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - SNE Vx, byte');

        if (this.V[x] != kk) {
          this.pc += 2;
        }

        this.pc += 2;
        break;

      case 0x5000:
        // 5xy0 - SE Vx, Vy
        // Skip next instruction if Vx = Vy.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - SE Vx, Vy');

        if (this.V[x] === this.V[y]) {
          this.pc += 2;
        }

        this.pc += 2;
        break;

      case 0x6000:
        // 6xkk - LD Vx, byte
        // Set Vx = kk.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - LD Vx, byte');

        this.V[x] = kk;

        this.pc += 2;
        break;

      case 0x7000:
        // 7xkk - ADD Vx, byte
        // Set Vx = Vx + kk.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - ADD Vx, byte');

        let temp = this.V[x] + kk;

        this.V[x] = temp;

        this.pc += 2;
        break;

      case 0x8000:
        switch (opcode & 0x000F) {
          case 0x0000:
            // 8xy0 - LD Vx, Vy
            // Set Vx = Vy.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - LD Vx, Vy');

            this.V[x] = this.V[y];

            this.pc += 2;
            break;

          case 0x0001:
            // 8xy1 - OR Vx, Vy
            // Set Vx = Vx OR Vy.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - OR Vx, Vy');

            this.V[x] = this.V[x] | this.V[y];

            this.pc += 2;
            break;

          case 0x0002:
            // 8xy2 - AND Vx, Vy
            // Set Vx = Vx AND Vy.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - AND Vx, Vy');

            this.V[x] = this.V[x] & this.V[y];

            this.pc += 2;
            break;

          case 0x0003:
            // 8xy3 - XOR Vx, Vy
            // Set Vx = Vx XOR Vy.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - XOR Vx, Vy');

            this.V[x] = this.V[x] ^ this.V[y];

            this.pc += 2;
            break;

          case 0x0004:
            // 8xy4 - ADD Vx, Vy
            // Set Vx = Vx + Vy, set VF = carry.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - ADD Vx, Vy');

            this.V[x] += this.V[y];

            // Carry bit
            if (this.V[x] > 0xFF) {
              this.V[0xF] = 1;
            } else {
              this.V[0xF] = 0;
            }

            this.pc += 2;
            break;

          case 0x0005:
            // 8xy5 - SUB Vx, Vy
            // Set Vx = Vx - Vy, set VF = NOT borrow.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - SUB Vx, Vy');

            if (this.V[x] < this.V[y]) {
              this.V[0xF] = 1;
            } else {
              this.V[0xF] = 0;
            }

            this.V[x] -= this.V[y];

            this.pc += 2;
            break;

          case 0x0006:
            // 8xy6 - SHR Vx {, Vy}
            // Set Vx = Vx SHR 1.
            // Shifts VY right by one and stores the result to VX (VY remains unchanged). VF is set to the value of the least significant bit of VY before the shift.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - SHR Vx');

            // this.V[0xF] = this.V[x] & 1;
            // this.V[x] /= 2;

            this.V[0xF] = this.V[x] & 0x01;
            this.V[x] = this.V[x] >> 1;

            this.pc += 2;
            break;

          case 0x0007:
            // 8xy7 - SUBN Vx, Vy
            // Set Vx = Vy - Vx, set VF = NOT borrow.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - SUBN Vx, Vy');

            if (this.V[y] < this.V[x])
              this.V[0xF] = 1;
            else
              this.V[0xF] = 0;

            this.V[x] = this.V[y] - this.V[x];

            this.pc += 2;
            break;

          case 0x000E:
            // 8xyE - SHL Vx {, Vy}
            // Set Vy = Vx = Vy SHL 1.
            // Shifts VY left by one and copies the result to VX. VF is set to the value of the most significant bit of VY before the shift.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - SHL Vx {, Vy}');

            // this.V[0xF] = this.V[y] >> 7;
            // this.V[x] = this.V[x] * 2;

            if (this.V[x] & 0x80) {
              this.V[0xF] = 1;
            } else {
              this.V[0xF] = 0;
            }

            this.V[x] = this.V[x] << 1;
            
            if (this.V[x] > 0xFF) {
              this.V[x] -= 0xF00;
            }

            this.pc += 2;
            break;

          default:
            this.log(LogLevels.ERROR, '[Chip8] emulateCycle - Unknown opcode: 0x' + leftPad(opcode.toString(16), 4, 0));
            this.pc += 2;
            break;
        }
        break;

      case 0x9000:
        // 9xy0 - SNE Vx, Vy
        // Skip next instruction if Vx != Vy.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - SNE Vx, Vy');

        if (this.V[x] != this.V[y]) {
          this.pc += 2;
        }

        this.pc += 2;
        break;

      case 0xA000:
        // Annn - LD I, addr
        // Set I = nnn.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - LD I, addr');

        this.I = nnn;

        this.pc += 2;
        break;

      case 0xB000:
        // Bnnn - JP V0, addr
        // Jump to location nnn + V0.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - JP V0, addr');

        this.pc = nnn + this.V[0x0];

        break;

      case 0xC000:
        // Cxkk - RND Vx, byte
        // Set Vx = random byte AND kk.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - RND Vx, byte');

        this.V[x] = (Math.floor(Math.random() * 0xFF)) & kk;

        this.pc += 2;
        break;

      case 0xD000:
        // Dxyn - DRW Vx, Vy, nibble
        // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - DRW Vx, Vy, nibble');

        const spriteHeight = opcode & 0x000F;

        let pixel;

        // Reset VF. We'll set this to 1 later if there is a collision.
        this.V[0xF] = 0;

        // For every line on the y axis
        for (let yline = 0; yline < spriteHeight; yline++) {
          // Get the pixel data from memory
          pixel = this.memory[this.I + yline];
          for (let xline = 0; xline < 8; xline++) {
            // 0x80 == 0b10000000
            // For each x value, check if it is to be toggled (pixel AND current value from mem != 0)
            if ((pixel & 0x80) > 0) {
              // If a pixel is to be toggled
              // Check if if the value is already on
              if (this.setPixel(this.V[x] + xline, this.V[y] + yline)) {
                // If it is already on, set VF to 1
                this.V[0xF] = 1;
              }
            }
            pixel <<= 1;
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
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - SKP Vx');

            if (this.keyboard[this.V[x]]) {
              this.pc += 2;
            }

            this.pc += 2;
            break;

          case 0x00A1:
            // ExA1 - SKNP Vx
            // Skip next instruction if key with the value of Vx is not pressed.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - SKNP Vx');

            if (!this.keyboard[this.V[x]]) {
              this.pc += 2;
            }

            this.pc += 2;
            break;

          default:
            this.log(LogLevels.ERROR, '[Chip8] emulateCycle - Unknown opcode: 0x' + leftPad(opcode.toString(16), 4, 0));
            this.pc += 2;
            break;
        }
        break;

      case 0xF000:
        switch (opcode & 0x00FF) {
          case 0x0007:
            // Fx07 - LD Vx, DT
            // Set Vx = delay timer value.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - LD Vx, DT');

            this.V[x] = this.delay_timer;

            this.pc += 2;
            break;

          case 0x000A:
            // Fx0A - LD Vx, K
            // Wait for a key press, store the value of the key in Vx.

            let keyPressed = false;

            for (let i = 0; i < 16; i++) {
              if (this.keyboard[i]) {
                this.V[x] = i;
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
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - LD DT, Vx');

            this.delay_timer = this.V[x];

            this.pc += 2;
            break;

          case 0x0018:
            // Fx18 - LD ST, Vx
            // Set sound timer = Vx.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - LD ST, Vx');

            this.sound_timer = this.V[x];

            this.pc += 2;
            break;

          case 0x001E:
            // Fx1E - ADD I, Vx
            // Set I = I + Vx.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - ADD I, Vx');

            this.I += this.V[x];

            if (this.I > 0xFFF) {
              this.V[0xF] = 1;
            } else {
              this.V[0xF] = 0;
            }

            // Handle overflow
            this.I = this.I & 0xFFF;

            this.pc += 2;
            break;

          case 0x0029:
            // Fx29 - LD I, Vx
            // Set I = location of sprite for digit Vx.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - LD I, Vx');

            this.I = this.V[x] * 5;

            this.pc += 2;
            break;

          case 0x0033:
            // Fx33 - LD B, Vx
            // Store BCD representation of Vx in memory locations I, I+1, and I+2.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - LD B, Vx');

            this.memory[this.I] = this.V[x] / 100;
            this.memory[this.I + 1] = (this.V[x] / 10) % 10;
            this.memory[this.I + 2] = (this.V[x] % 100) % 10;

            this.pc += 2;
            break;

          case 0x0055:
            // Fx55 - LD [I], Vx
            // Store registers V0 through Vx in memory starting at location I.
            // Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - LD [I], Vx');

            // For each register
            for (let a = 0; a <= (x); a++) {
              // Save the register's data
              this.memory[this.I + a] = this.V[a];
            }

            // This conflicts with Wikipedia's description but matches BYTE Magazine Vol 3 Num 12 p110
            // this.I += (x) + 1;

            this.pc += 2;
            break;

          case 0x0065:
            // Fx65 - LD Vx, [I]
            // Read registers V0 through Vx from memory starting at location I.
            // Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
            this.log(LogLevels.DEBUG, '[Chip8] opcode 0x' + leftPad(opcode.toString(16), 4, 0) + ' - LD Vx, [I]');

            // For each register
            for (let a = 0; a <= (x); a++) {
              // Load memory into the register
              this.V[a] = this.memory[this.I + a];
            }

            // This conflicts with Wikipedia's description but matches BYTE Magazine Vol 3 Num 12 p110
            // this.I += (x) + 1;

            this.pc += 2;
            break;

          default:
            this.log(LogLevels.ERROR, '[Chip8] emulateCycle - Unknown opcode: 0x' + leftPad(opcode.toString(16), 4, 0));
            this.pc += 2;
            break;
        }
        break;

      default:
        this.log(LogLevels.ERROR, '[Chip8] emulateCycle - Unknown opcode: 0x' + leftPad(opcode.toString(16), 4, 0));
        this.pc += 2;
        break;
    }

    // Handle program counter overflow
    this.pc &= 0x0FFF;
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
  0xF0, 0x80, 0xF0, 0x80, 0x80, // F
];
