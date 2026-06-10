// May 09, 2026
#pragma once

#include <cctype> // for character classification functions like isdigit() to parse counts and isspace() for word jumping.
#include <cstddef> // for fundamental types and sizes, specifically size_t for character indices and buffer offsets.
#include <cstring> // for low-level string manipulation utilities like strlen() or raw memory block copies (memcpy).
#include <filesystem> // for validating file existence, checking directory permissions, and extracting clean file pathways.
#include <fstream> // for opening, reading, and executing raw file streaming modifications to disk via ifstream and ofstream.
#include <iomanip> // for text alignment controls and padding formatters like setw() to draw structured row numbers.
#include <ios> // for core input/output stream base properties and configuration constants like std::ios::binary.
#include <iostream> // for standard character streams like cin and cout to interact with console inputs and display outputs.
#include <iterator> // for iterator tags and properties used to step or advance through custom memory arrays.
#include <sstream> // for string streams to easily tokenize command strings or build text messages dynamically.
#include <stdlib.h> // for legacy memory allocation control and system commands, such as exiting the process.
#include <string> // for using the std::string class to store, manipulate, and hold lines of editable text payloads.
#include <sys/ioctl.h> // for interacting with terminal hardware settings, specifically fetching the current window row/column size.
#include <termios.h> // for modifying terminal configuration flags to put the console into raw mode and disable character echo.
#include <unistd.h> // for standard POSIX system abstractions, primarily reading file descriptors and writing raw escape sequences.
#include <vector> // for standard dynamic arrays (though being factored out to maintain your from-scratch architecture).

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::right;   // For aligning text to the right
using std::setfill; // For specifying a padding character
using std::setw;    // For configuring the maximum character width of the next
using std::string;  // For managing dynamic text blocks, tracking single lines,
using std::stringstream; // For handling memory-buffered streams to
using std::to_string; // For converting numerical indices, column targets, and
using std::vector;    // For managing standard dynamic contiguous memory arrays

namespace AnsiSeq {
inline string clearScreen() { return "\x1b[2J"; }
inline string clearLine() { return "\x1b[K"; }
inline string altBufferOn() { return "\x1b[?1049h"; }
inline string altBufferOff() { return "\x1b[?1049l"; }
inline string hideCursor() { return "\x1b[?25l"; }
inline string showCursor() { return "\x1b[?25h"; }

inline string moveCursor(size_t row, size_t col) {
  return "\x1b[" + to_string(row) + ";" + to_string(col) + "H";
}
// Highlight colors
inline string searchHighlight() {
  // CHANGED: Cyan background instead of yellow
  return "\x1b[48;5;51m\x1b[38;5;16m";
} // Bright cyan background, black text

/* OTHER COLOR OPTIONS for searchHighlight():
   Yellow (original):     "\x1b[48;5;220m\x1b[38;5;16m"  (yellow bg, black text)
   Green:                 "\x1b[48;5;46m\x1b[38;5;16m"   (bright green bg, black
   text) Red:                   "\x1b[48;5;196m\x1b[38;5;255m" (bright red bg,
   white text) Blue:                  "\x1b[48;5;21m\x1b[38;5;255m"  (bright
   blue bg, white text) Magenta:               "\x1b[48;5;201m\x1b[38;5;255m"
   (magenta bg, white text) Orange:                "\x1b[48;5;208m\x1b[38;5;16m"
   (orange bg, black text) White:                 "\x1b[48;5;255m\x1b[38;5;16m"
   (white bg, black text) Dark gray:             "\x1b[48;5;240m\x1b[38;5;255m"
   (dark gray bg, white text)
*/
inline string reset() { return "\x1b[0m"; }
} // namespace AnsiSeq