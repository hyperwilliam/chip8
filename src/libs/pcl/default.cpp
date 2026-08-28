// Platform Compatibility Layer.
// "Platform Compatibility Layer For Raylib"
// the main.cpp file handles backend, this is the frontend.
// however, there is only stubs here (for now!)
// TODO: Stubs should be written to call raylib functions
#include "raylib.h"
#include <iostream>
#include <fstream>
#include <filesystem>
using namespace std;
// PCL Specific

bool expectPCLVersion(const char *version) {
  return version == "1.0.0";
}

const char* getPlatform() {
  return "PCL 1.0.0-RLIB";
}
// Graphics Layer
bool createWindow(int xSize, int ySize, const char *text) { // returns true if success
  InitWindow(xSize,ySize,text);
  if (!IsWindowReady()) {
    return false;
  }
  SetTargetFPS(60);
  return true;
}

void destroyWindow() {
  CloseWindow();
  return;
}

void renameWindow(const char *text) {
  SetWindowTitle(text);
  return;
}

bool ifWinClose() {
  return !WindowShouldClose();
}

void drawStart() { // not sure how this would work on non-raylib... (im sure it'll be fine)
  BeginDrawing();
  return;
}
void drawEnd() { // not sure how this would work on non-raylib...
  EndDrawing();
  return;
}
void print(const char *text, int posX, int posY, int fontSize, Color color) {
  DrawText(text, posX, posY, fontSize, color);
  return;
}
void clearBG(Color color) {
  ClearBackground(color);
  return;
}
void plotPixel(int posX, int posY, Color color) {
  DrawPixel(posX, posY, color); // supposedly "Slow".
  return;
}

void plotRect(int x, int y, int width, int height, Color color) {
  DrawRectangle(x,y,width,height,color);
  return;
}
float getDelta() {
  return GetFrameTime();
}

// File Layer
bool fsExists(const char *fileName) {
  return FileExists(fileName);
}
unsigned long fsSize(const char *fileName) {
  return filesystem::file_size(fileName);
}
unsigned char fsByte(const char *fileName, int location) {
  fstream file(fileName, ios_base::binary);
  file.open(fileName);
  if (!file.is_open()) {
    cerr << "Error: Unable to open file!" << endl;
    return 1;
  }
  file.seekg(location);
  unsigned int val = file.peek();
  if (val == 0xFFFFFFFF) { // this is an anomaly in the peek function (and presumably others) that happens when reading 0x1A (or EOF)
    val = 0x1A;
  }
  file.close();
  return (unsigned char) val;
}

unsigned char* fsOpen(const char *fileName, unsigned int bytes, unsigned long offset) {
  fstream file(fileName, ios_base::binary);
  file.open(fileName);
  file.seekg(offset);
  char temp[bytes];
  unsigned char* bytesVal = new unsigned char[bytes];
  unsigned int i = 0;
  file.read(temp, bytes);
  for (;i < bytes; i++) {
    bytesVal[i] = (unsigned char)temp[i];
  }
  file.close();
  return bytesVal;
}

unsigned char* fsReadUntilNull(const char *fileName, unsigned long offset) {
  fstream file(fileName, ios_base::binary);
  file.open(fileName);
  file.seekg(offset);
  char temp[255];
  unsigned char* bytesVal = new unsigned char[255];
  unsigned char i = 0;
  file.get(temp, 255);
  while (temp[i] != 0) {
    bytesVal[i] = (unsigned char)temp[i];
    i++;
  }
  bytesVal[i] = 0;
  file.close();
  return bytesVal;
}

// Terminal Layer (will be a stub on some systems that wont work with this)
void printInfo(const char *text) {
  TraceLog(LOG_INFO, text);
  return;
}
void printWarn(const char *text) {
  TraceLog(LOG_WARNING, text);
  return;
}
void printError(const char *text) {
  TraceLog(LOG_ERROR, text);
  return;
}
