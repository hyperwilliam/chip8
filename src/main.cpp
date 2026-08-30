#include "libs/pcl/default.cpp" // Platform Compatibility Layer. Adjust for system target
#include <cstdlib>
struct pc {
  unsigned char v[16];
  unsigned char ram[4096];
  bool frame[2048];
  unsigned int stack[16];
  unsigned char sp;
  unsigned int pc;
  unsigned int index;
  unsigned char delay;
  unsigned char sound;
  unsigned char waitforKey;
  bool keys[16];
  bool crash;
}chip;

bool getBit(unsigned int number, unsigned char position) {
  return ((number >> position) & 1) == 1;
}
unsigned int read16(int address, pc memory) {
  return memory.ram[address + 1] + (memory.ram[address] << 8);
}

unsigned char read(int address, pc memory) {
  return memory.ram[address];
}

void setKeys(pc &chip) {
  for (unsigned char i = 0; i < 16; i++)
    chip.keys[i] = keyGetD(i);
  return;
}

unsigned char parseOpc(pc &chip) {
  unsigned int opcode = read16(chip.pc, chip);
  unsigned int opcodeH = (opcode & 0xF000) >> 12;
  unsigned int opcodeHL = (opcode & 0xF00) >> 8;
  unsigned int opcodeLH = (opcode & 0xF0) >> 4;
  unsigned int opcodeL = (opcode & 0xF);
    cout << "PC: " << chip.pc << "\n" << "OPCODE: " << opcode << "\n";
  if (opcode == 0x00E0) {
    for (unsigned int i = 0; i < 2048; i++) {
      chip.frame[i] = false;
    }
    return 0;
  }
  if (opcode == 0x00EE) {
    chip.sp += 1;
    chip.sp &= 15;
    chip.pc = chip.stack[chip.sp];
    return 0;
  }
  if (opcodeH == 0xF) {
    if ((opcode & 0xFF) == 0x65) {
      for (unsigned char i = 0; i < opcodeHL + 1; i++) {
        chip.v[i] = chip.ram[(chip.index + i) & 0xFFF];
      }
      chip.index += opcodeHL;
      return 0;
    }
    if ((opcode & 0xFF) == 0x55) {
      for (unsigned char i = 0; i < opcodeHL + 1; i++) {
        chip.ram[(chip.index + i) & 0xFFF] = chip.v[i];
      }
      chip.index += opcodeHL;
      return 0;
    }

    if ((opcode & 0xFF) == 0x33) {
      chip.ram[chip.index + 2] = chip.v[opcodeHL] % 10;
      chip.ram[chip.index + 1] = (chip.v[opcodeHL] / 10) % 10;
      chip.ram[chip.index] = (chip.v[opcodeHL] / 100) % 10;
      return 0;
    }
    if ((opcode & 0xFF) == 0x29) {
      chip.index = 0x50 + (chip.v[opcodeHL] * 5);
      return 0;
    }
    if ((opcode & 0xFF) == 0x1E) {
      chip.index += chip.v[opcodeHL];
      chip.pc &= 0xFFF;
      return 0;
    }
    if ((opcode & 0xFF) == 0x18) {
      chip.sound = chip.v[opcodeHL];
      return 0;
    }
    if ((opcode & 0xFF) == 0x15) {
      chip.delay = chip.v[opcodeHL];
      return 0;
    }
    if ((opcode & 0xFF) == 0x0A) {
      chip.waitforKey = opcodeHL;
      return 0;
    }
    if ((opcode & 0xFF) == 0x07) {
      chip.v[opcodeHL] = chip.delay;
      return 0;
    }
  }
  if (opcodeH == 0xE) {
    if ((opcode & 0xFF) == 0x9E) {
      if (chip.keys[chip.v[opcodeHL]]) {
        chip.pc += 2;
      }
      return 0;
    }
    if ((opcode & 0xFF) == 0xA1) {
      if (!chip.keys[chip.v[opcodeHL]]) {
        chip.pc += 2;
      }
      return 0;
    }
  }
  if (opcodeH == 0xD) {
    unsigned int drawPtr = 0;
    chip.v[0xF] = 0;
    for (unsigned char byte = 0; byte < opcodeL; byte++) {
      unsigned char byteVal = read(chip.index + byte, chip);
      for (unsigned char bit = 0; bit < 8; bit++) {
        drawPtr = ((chip.v[opcodeHL] + bit) % 64) + (((chip.v[opcodeLH] + byte) % 32) * 64);
        if (chip.frame[drawPtr] == 1) {
          chip.v[0xF] = 1;
        }
        chip.frame[drawPtr] ^= getBit(byteVal, 7-bit);
      }
    }
    return 0xFF;
  }
  if (opcodeH == 0xC) {
    chip.v[opcodeHL] = rand() % ((opcode & 0xFF) + 1);
    return 0;
  }
  if (opcodeH == 0xB) {
    chip.pc = ((opcode & 0xFFF) + chip.v[0]) - 2;
    chip.pc &= 0xFFF;
    return 0;
  }
  if (opcodeH == 0xA) {
    chip.index = (opcode & 0xFFF);
    return 0;
  }
