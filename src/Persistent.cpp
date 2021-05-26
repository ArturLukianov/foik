#include "main.hpp"

Saver::Saver() {
}

Saver::~Saver() {}

bool Saver::fileExists(const char *path) {
  if (FILE *file = fopen(path, "r")) {
    fclose(file);
    return true;
  } else {
    return false;
  }   
}

void Saver::deleteFile(const char *path) {
  remove(path);
}

void Saver::Saver::putInt(int val) {
  buf.write((const char *)&val, sizeof(val));
}

void Saver::putChar(char val) {
  buf.write((const char *)&val, sizeof(val));
}

void Saver::putColor(const TCODColor *val) {
  putChar(val->r);
  putChar(val->g);
  putChar(val->b);
}

void Saver::putString(const char *val) {
  int length = strlen(val);
  putInt(length);
  buf.write(val, length);
}

void Saver::putFloat(float val) {
  buf.write((const char *)&val, sizeof(val));
}

void Saver::saveToFile(const char *path) {
  std::ofstream output;
  output.open(path);
  output << buf.rdbuf();
  output.close();
}


int Saver::getInt() {
  int val;
  buf.read((char *)&val, sizeof(val));
  return val;
}

char Saver::getChar() {
  char val;
  buf.read((char *)&val, sizeof(val));
  return val;
}

TCODColor Saver::getColor() {
  char r = getChar();
  char g = getChar();
  char b = getChar();
  return TCODColor(r, g, b);
}

const char * Saver::getString() {
  int length = getInt();
  char * val = new char[length+1];
  buf.read((char *)val, length);
  val[length] = 0;
  return val;
}

float Saver::getFloat() {
  float val;
  buf.read((char *)&val, sizeof(val));
  return val;
}

void Saver::loadFromFile(const char * path) {
  std::ifstream input;
  input.open(path);
  buf << input.rdbuf();
  buf.seekg(0);
  input.close();
}
