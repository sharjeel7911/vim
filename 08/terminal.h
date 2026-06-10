#include "utilities.h"
#include "editor.h"
#include "gapbuffer.h"

struct AppendBuffer {
  string buff;
  void append(const string& str) { buff += str; }
  void append(const char* str, int len) { buff.append(str, len); }
};

#ifndef TERMINAL_H
#define TERMINAL_H
class Terminal {
private:
  struct termios orig_termios;
  size_t screenRows;
  size_t screenCols;
public:
  Terminal();
  ~Terminal();

  void enableRawMode();
  void disableRawMode();
  void getWindowSize();

  size_t getScreenRows();
  size_t getScreenCols();

  InputKey readKey();
  void render(TextEditor&);
};
#endif