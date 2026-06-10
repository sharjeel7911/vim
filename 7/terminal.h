// #ifndef TERMINAL_H
// #define TERMINAL_H

// #include "editor.h"
// #include "gapbuffer.h"
// #include <string>

// namespace AnsiSeq {
// // Structural commands
// inline string clearScreen() { return "\x1b[2J"; }
// inline string clearLine() { return "\x1b[K"; }
// inline string altBufferOn() { return "\x1b[?1049h"; }
// inline string altBufferOff() { return "\x1b[?1049l"; }
// inline string hideCursor() { return "\x1b[?25l"; }
// inline string showCursor() { return "\x1b[?25h"; }

// inline string moveCursor(size_t row, size_t col) {
//   return "\x1b[" + to_string(row) + ";" + to_string(col) + "H";
// }
// } // namespace AnsiSeq

// namespace Style {
// const string RESET = "\x1b[0m";
// const string BOLD = "\x1b[1m";
// const string ITALIC = "\x1b[3m";
// } // namespace Style

// inline string bg256(int id) { return "\x1b[48;5;" + to_string(id) + "m"; }
// inline string fg256(int id) { return "\x1b[38;5;" + to_string(id) + "m"; }

// struct AppendBuffer {
//   string buff;
//   void append(const string &str) { buff += str; }
//   void append(const char *str, int len) { buff.append(str, len); }
// };

// class Terminal {
// private:
//   struct termios orig_termios;
//   size_t screenRows; // Total height of terminal
//   size_t screenCols; // Total width of terminal

//   void die(const char *);

// public:
//   Terminal();
//   ~Terminal();

//   void enableRawMode();
//   void disableRawMode();
//   void getWindowSize(); // Dynamic window size lookup

//   // Getters for TextEditor to know the limits
//   size_t getScreenRows();
//   size_t getScreenCols();

//   void render(TextEditor &);
//   InputKey readKey();

//   void drawFileLines(AppendBuffer &, TextEditor &, int, int, int &, int &);
//   void drawStatusBars(AppendBuffer &, TextEditor &);
// };

// #endif

// // void SetColor(size_t textColor) { cout << "\033[" << textColor << "m"; }

// // void ResetColor() { cout << "\033[0m"; }

// // size_t row = 24;

// // // Move cursor
// //  cout << "\033[" << row << ";1H";

// // // Background + text color
// //  cout << "\033[48;2;50;50;50m";
// //  cout << "\033[38;2;255;255;255m";

// // // Italic
// //  cout << "\033[3m";

// //  cout << " NORMAL | file.cpp | Ln 10, Col 5 ";

// // // Clear remaining line
// //  cout << "\033[K";

// // // Reset
// //  cout << "\033[0m";

// // cin.get();

#ifndef TERMINAL_H
#define TERMINAL_H

#include "editor.h"
#include "gapbuffer.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

// POSIX/Unix System Headers to clear Clang/Zed errors
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

using std::right;
using std::setfill;
using std::setw;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

namespace Style {
const string RESET = "\x1b[0m";
const string BOLD = "\x1b[1m";
const string ITALIC = "\x1b[3m";
} // namespace Style

inline string bg256(int id) { return "\x1b[48;5;" + to_string(id) + "m"; }
inline string fg256(int id) { return "\x1b[38;5;" + to_string(id) + "m"; }

struct AppendBuffer {
  string buff;
  void append(const string &str) { buff += str; }
  void append(const char *str, int len) { buff.append(str, len); }
};

class Terminal {
private:
  struct termios orig_termios;
  size_t screenRows;
  size_t screenCols;

  void die(const char *);

public:
  Terminal();
  ~Terminal();

  void enableRawMode();
  void disableRawMode();
  void getWindowSize();

  size_t getScreenRows();
  size_t getScreenCols();

  void render(TextEditor &);
  InputKey readKey();
};

#endif