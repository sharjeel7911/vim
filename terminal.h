#include "editor.h"
#include "gapbuffer.h"

// void SetColor(size_t textColor) { cout << "\033[" << textColor << "m"; }

// void ResetColor() { cout << "\033[0m"; }

// size_t row = 24;

// // Move cursor
// std::cout << "\033[" << row << ";1H";

// // Background + text color
// std::cout << "\033[48;2;50;50;50m";
// std::cout << "\033[38;2;255;255;255m";

// // Italic
// std::cout << "\033[3m";

// std::cout << " NORMAL | file.cpp | Ln 10, Col 5 ";

// // Clear remaining line
// std::cout << "\033[K";

// // Reset
// std::cout << "\033[0m";

// cin.get();

struct AppendBuffer {
  string buff;
  void append(const string &s) { buff += s; }
  void append(const char *s, int len) { buff.append(s, len); }
};

#ifndef TERMINAL_H
#define TERMINAL_H

class Terminal {
private:
  struct termios orig_termios;
  size_t screenRows; // Total height of terminal
  size_t screenCols; // Total width of terminal

  void die(const char *);

public:
  Terminal();
  ~Terminal();

  void enableRawMode();
  void disableRawMode();
  void getWindowSize(); // Dynamic window size lookup

  // Getters for TextEditor to know the limits
  size_t getScreenRows();
  size_t getScreenCols();

  void render(TextEditor &);
  InputKey readKey();
};

#endif
