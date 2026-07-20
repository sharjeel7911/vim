// May 09, 2026
#pragma once

#include <cctype> 
#include <cstddef> 
#include <cstring>
#include <filesystem>
#include <fstream> 
#include <iomanip>
#include <ios> 
#include <iostream>
#include <iterator> 
#include <sstream>
#include <stdlib.h> 
#include <string> 
#include <sys/ioctl.h>
#include <termios.h> 
#include <unistd.h> 
#include <vector> 

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::right;
using std::setfill;
using std::setw;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

/* --------------------------------------------------------------------------------------- */

namespace AnsiSeq {
  inline string clearScreen() { return "\x1b[2J"; }
  inline string clearLine() { return "\x1b[K"; }
  inline string altBufferOn() { return "\x1b[?1049h"; }
  inline string altBufferOff() { return "\x1b[?1049l"; }
  inline string hideCursor() { return "\x1b[?25l"; }
  inline string showCursor() { return "\x1b[?25h"; }
  inline string reset() { return "\x1b[0m"; }
  inline string moveCursor(size_t row, size_t col) {
    return "\x1b[" + to_string(row) + ";" + to_string(col) + "H";
  }
  inline string searchHighlight() {
    return "\x1b[48;5;51m\x1b[38;5;16m";
  }
}

namespace Style {
  const string RESET = "\x1b[0m";
  const string BOLD = "\x1b[1m";
  const string ITALIC = "\x1b[3m";
}

inline string bg256(int id) { return "\x1b[48;5;" + to_string(id) + "m"; }
inline string fg256(int id) { return "\x1b[38;5;" + to_string(id) + "m"; }
