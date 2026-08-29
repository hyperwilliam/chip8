#include "libs/pcl/default.cpp" // Platform Compatibility Layer. Adjust for system target
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

void parseOpc(pc &chip) {
  unsigned int opcode = read16(chip.pc, chip);
  unsigned int opcodeH = (opcode & 0xF000) >> 12;
  unsigned int opcodeHL = (opcode & 0xF00) >> 8;
  unsigned int opcodeLH = (opcode & 0xF0) >> 4;
  unsigned int opcodeL = (opcode & 0xF);
  if (opcode == 0x00E0) {
    for (unsigned int i = 0; i < 2048; i++) {
      chip.frame[i] = false;
    }
    return;
  }
  if (opcode == 0x00EE) {
    chip.sp += 1;
    chip.sp &= 15;
    chip.pc = chip.stack[chip.sp];
    return;
  }
  if (opcodeH == 0xF) {
    if ((opcode & 0xFF) == 0x65) {
      for (unsigned char i = 0; i < opcodeHL + 1; i++) {
        chip.v[i] = chip.ram[(chip.index + i) & 0xFFF];
      }
      return;
    }
    if ((opcode & 0xFF) == 0x55) {
      for (unsigned char i = 0; i < opcodeHL + 1; i++) {
        chip.ram[(chip.index + i) & 0xFFF] = chip.v[i];
      }
      return;
    }

    if ((opcode & 0xFF) == 0x33) {
      chip.ram[chip.index + 2] = chip.v[opcodeHL] % 10;
      chip.ram[chip.index + 1] = (chip.v[opcodeHL] / 10) % 10;
      chip.ram[chip.index] = (chip.v[opcodeHL] / 100) % 10;
      return;
    }
    if ((opcode & 0xFF) == 0x1E) {
      chip.index += chip.v[opcodeHL];
      chip.pc &= 0xFFF;
      return;
    }
    if ((opcode & 0xFF) == 0x15) {
      cout << (unsigned int)chip.v[opcodeHL] << " " << (unsigned int)chip.delay << "\n";
      chip.delay = chip.v[opcodeHL];
      return;
    }
    if ((opcode & 0xFF) == 0x07) { // its a bit, broken ...
      cout << (unsigned int)chip.v[opcodeHL] << " " << (unsigned int)chip.delay << "\n";
      chip.v[opcodeHL] = chip.delay;
      return;
    }
  }
  if (opcodeH == 0xE) {
    if ((opcode & 0xFF) == 0x9E) {
      if (chip.keys[chip.v[opcodeHL]]) {
        chip.pc += 2;
      }
      return;
    }
    if ((opcode & 0xFF) == 0xA1) {
      if (!chip.keys[chip.v[opcodeHL]]) {
        chip.pc += 2;
      }
      return;
    }
  }
  if (opcodeH == 0xD) {
    unsigned int drawPtr = 0;
    chip.v[0xF] = 0;
    for (unsigned char byte = 0; byte < opcodeL; byte++) {
      unsigned char byteVal = read(chip.index + byte, chip);
      for (unsigned char bit = 0; bit < 8; bit++) {
        drawPtr = (chip.v[opcodeHL] & 63) + ((chip.v[opcodeLH] + byte) * 64) + bit;
        if (chip.frame[drawPtr] == 1) {
          chip.v[0xF] = 1;
        }
        chip.frame[drawPtr] ^= getBit(byteVal, 7-bit);
        //cout << "x";
      }
    }
    return;
  }
  if (opcodeH == 0xB) {
    chip.pc = ((opcode & 0xFFF) + chip.v[0]) - 2;
    chip.pc &= 0xFFF;
    return;
  }
  if (opcodeH == 0xA) {
    chip.index = (opcode & 0xFFF);
    return;
  }
  if (opcodeH == 9) {
    if (chip.v[opcodeHL] != chip.v[opcodeLH]) {
      chip.pc += 2;
    }
    return;
  }
  if (opcodeH == 8) {
    bool setVF = false;
    if (opcodeL == 0) {
      chip.v[opcodeHL] = chip.v[opcodeLH];
      return;
    }
    if (opcodeL == 1) {
      chip.v[opcodeHL] |= chip.v[opcodeLH];
      return;
    }
    if (opcodeL == 2) {
      chip.v[opcodeHL] &= chip.v[opcodeLH];
      return;
    }
    if (opcodeL == 3) {
      chip.v[opcodeHL] ^= chip.v[opcodeLH];
      return;
    }
    if (opcodeL == 4) {
      setVF = (chip.v[opcodeHL] + chip.v[opcodeLH]) > 255;
      chip.v[opcodeHL] += chip.v[opcodeLH];
      chip.v[0xF] = (unsigned char)setVF;
      return;
    }
    if (opcodeL == 5) {
      setVF = (chip.v[opcodeHL] < chip.v[opcodeLH]);
      chip.v[opcodeHL] -= chip.v[opcodeLH];
      chip.v[0xF] = (unsigned char)!setVF;
      return;
    }
    if (opcodeL == 6) {
      setVF = (chip.v[opcodeHL] & 1) == 1;
      chip.v[opcodeHL] >>= 1;
      chip.v[0xF] = (unsigned char)setVF;
      return;
    }
    if (opcodeL == 7) {
      setVF = (chip.v[opcodeHL] > chip.v[opcodeLH]);
      chip.v[opcodeHL] = chip.v[opcodeLH] - chip.v[opcodeHL];
      chip.v[0xF] = (unsigned char)!setVF;
      return;
    }
    if (opcodeL == 14) {
      setVF = (chip.v[opcodeHL] & 0x80) == 0x80;
      chip.v[opcodeHL] <<= 1;
      chip.v[0xF] = (unsigned char)setVF;
      return;
    }
  }
  if (opcodeH == 7) {
    chip.v[opcodeHL] += (opcode & 0xFF);
    return;
  }
  if (opcodeH == 6) {
    chip.v[opcodeHL] = (opcode & 0xFF);
    return;
  }
  if (opcodeH == 5) {
    if (chip.v[opcodeHL] == chip.v[opcodeLH]) {
      chip.pc += 2;
    }
    return;
  }
  if (opcodeH == 4) {
    if (chip.v[opcodeHL] != (opcode & 0xFF)) {
      chip.pc += 2;
    }
    return;
  }
  if (opcodeH == 3) {
    if (chip.v[opcodeHL] == (opcode & 0xFF)) {
      chip.pc += 2;
    }
    return;
  }
  if (opcodeH == 2) {
    chip.stack[chip.sp] = chip.pc;
    chip.pc = (opcode & 0xFFF) - 2;
    chip.sp -= 1;
    chip.sp &= 15;
    return;
  }
  if (opcodeH == 1) {
    chip.pc = (opcode & 0xFFF) - 2;
    return;
  }
  printError("FATAL! Invalid Opcode Exception");
  cout << chip.pc << "\n" << opcode << "\n";
  chip.crash = true;
  return;
}

void drawScreen(unsigned int x, unsigned int y, bool color) {
  unsigned int xpos = x * 16;
  unsigned int ypos = y * 16;
  if (color) {
    plotRect(xpos, ypos, 16,16, (Color) {255,255,255,255});
  } else {
    plotRect(xpos, ypos, 16,16, (Color) {255,255,255,0});
  }
  return;
}
int main(int argc, char *argv[]) {
  const char *filename = "target.ch8";
  if (argc >= 2) {
    filename = argv[1];
  } else {
    filename = "target.ch8";
  }
  if (fsExists(filename)) {
    printInfo("ANALYSER Started!");
  } else {
    printError("Input File Was not Found!");
    return 0;
  }
  if (!createWindow(1024, 512, "Hyper's CHIP-8 Emulator!")) {
    printError("Window Creation Failed!");
    return 1;
  }
  printInfo("Window Created Successfully!");
  pc chip;
  for (unsigned int i = 0; i < 4096; i++) {
    chip.ram[i] = 0;
  }
  unsigned char* fsTemp = fsOpen(filename, fsSize(filename), 0);
  for (unsigned int i = 0; i < fsSize(filename); i++) {
    chip.ram[i + 0x200] = fsTemp[i];
  }
  for (unsigned int i = 0; i < 16; i++) {
    chip.v[i] = 0;
  }
  unsigned char font[80] =
  {
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80
  };
  for (unsigned int i = 0; i < 80; i++) {
    chip.ram[i + 0x50] = font[i];
  }
  chip.ram[0x1FF] = 1;
  chip.delay = 0;
  chip.sound = 0;
  chip.pc = 0x200;
  chip.sp = 15;
  chip.crash = false;
  while(ifWinClose()) {
    drawStart();
    clearBG(BLACK);
    for (unsigned char i = 0; i < 12; i++) {
      if (!chip.crash) {
        parseOpc(chip);
        chip.pc += 2;
        chip.pc &= 0xFFF;
      }
      if (chip.delay) {
        chip.delay -= 1;
      }
    }
    for (char y = 0; y < 32; y++) {
      for (char x = 0; x < 64; x++) {
        drawScreen(x, y, chip.frame[x + (y * 64)]);
      }
    }
    drawEnd();
  }
  return 0;
} 

