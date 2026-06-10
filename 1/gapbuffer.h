#include "utilities.h"

#ifndef GapBuffer_H
#define GapBuffer_H

class GapBuffer {
private:
  // +--------------------------------------------------------------------------------------------------+
  // Internal data
  // +--------------------------------------------------------------------------------------------------+
  char *buffer;
  size_t bufferSize;
  size_t gapStart, gapEnd;

  // +--------------------------------------------------------------------------------------------------+
  // Internal methods
  // +--------------------------------------------------------------------------------------------------+

public:
  // expands buffer when gap is too small, copy data preserving gap
  void regrowBuffer(size_t);
  // physically shift gap start to align with a new cursor position
  void moveGapTo(size_t);

  void display(); // For testing

  // +--------------------------------------------------------------------------------------------------+
  // Lifecycle
  // +--------------------------------------------------------------------------------------------------+

  // constructor: allocate buffer at start, set gapStart = 0, gapEnd = capacity
  // (whole buffer is gap)
  GapBuffer(size_t initialCap = 8);
  // destructor: free heap memory
  ~GapBuffer();

  // +--------------------------------------------------------------------------------------------------+
  // Editing
  // +--------------------------------------------------------------------------------------------------+

  // place char at gapStart
  void insertCharAtCursor(char);
  // insert every char in str at cursor
  void insertStringAtCursor(const string &);
  // insert string at arbitrary position by moving gap there first
  void insertStringAtPos(size_t, const string &);
  // backspace: decrement gapStart
  void deleteCharBeforeCursor();
  // delete: increment gapEnd
  void deleteCharAfterCursor();
  // delete text between two indices
  void deleteTextInRange(size_t, size_t);
  // replace text in a given range with new text
  void replaceTextInRange(size_t, size_t, const char *);
  // delete whole buffer content and reset gap to initial state
  void clearBuffer();

  // +--------------------------------------------------------------------------------------------------+
  // Cursor movement
  // +--------------------------------------------------------------------------------------------------+

  // slide one char from right side of gap to left (moves cursor forward)
  void moveCursorRight();
  // slide one char from left side of gap to right (moves cursor back)
  void moveCursorLeft();
  // jump cursor to same column on the previous line
  void moveCursorUp(size_t);
  // jump cursor to same column on the next line
  void moveCursorDown(size_t);
  // move gap to arbitrary index
  void moveCursorToPos(size_t);
  // for cursor to jump specific line like jump to line 110 (:110) : absolute
  // jump
  void moveCursorToSpecificRow(size_t);
  // skip over a whole word left/right -> for 'w' & 'b'
  void moveCursorByWord(bool);
  // go at end of current word -> for 'e'
  void moveCursorToEndOfWord();
  // scan left until '\n' or index 0
  void moveCursorToRowStart();
  // scan right until '\n' or end of text
  void moveCursorToRowEnd();
  // jump to position 0
  void moveCursorToBufferStart();
  // jump to last character position
  void moveCursorToBufferEnd();

  // +--------------------------------------------------------------------------------------------------+
  // Text access
  // +--------------------------------------------------------------------------------------------------+

  // return character at logical index without building full string
  char getCharAt(size_t);
  // extract a specific line by line number
  string getRowText(size_t);
  // concatenate whole buffer into one string
  string getWholeBufferText();
  // Extracts text for a given range and returns it as a string
  string getTextInRange(size_t, size_t);

  // +--------------------------------------------------------------------------------------------------+
  // State queries
  // +--------------------------------------------------------------------------------------------------+

  // checks whether a word is valid
  bool isWordChar(char);
  // return given index value
  char &operator[](size_t);
  // returns gapStart
  size_t getGapStart();
  // getTextLength() == 0 : nothing typed yet
  bool isBufferEmpty();
  // returns gapStart == gapEnd
  bool isGapFilled();
  // allocated size excluding the gap
  size_t getBufferCapacity();
  // number of actual characters in buffer excluding gap
  size_t getTextLength();
  // gapEnd - gapStart
  size_t getGapSize();
  // returns gapStart
  size_t getCursorPosition();
  // for status bar: total lines
  size_t getRowCount();
  // finds line number from a position
  size_t getRowIndexFromPos(size_t);
  // finds column number from a position in a row [we have to find the line
  // start first to calculate column as buffer don't know anything about
  // rows/columns, it just has a gap and chars]
  size_t getColIndexFromPos(size_t);
  // get current row of cursor
  size_t getCurrentRowIndex();
  // get current column of cursor
  size_t getCurrentColIndex();
  // get index of start of line
  size_t getRowStartIndex(size_t);
  // get index of end of line
  size_t getRowEndIndex(size_t);
  // for '/' search command, finds next occurrence of pattern starting from
  // index 'start' and returns its index or -1 if not found
  size_t findNextPatternFirstIndex(const char *, size_t);
  // for '?' search command, finds previous occurrence of pattern starting from
  // index 'start' and returns its index or -1 if not found
  size_t findPreviousPatternFirstIndex(const char *, size_t);
};

#endif

// =============================================================================

// , HashTable  ,   ,
// class Terminal {
// public:

//     void clearScreen();
//     // void drawLines(const MyVec<GapBuffer>& line   s, size_t currentLine,
//     size_t cursorInLine);
// };

// hash for keyword highlighting , command lookup
// getKeys() using raw escape sequences is actually the right Linux approach
// though — keep that idea The one thing your friend got right that you should
// steal: his getKeys() escape sequence parser for Linux arrow keys (\033[A =
// up, \033[B = down, etc.). That exact pattern is what you need on Ubuntu. Copy
// that logic directly.

// keys.h — same on both platforms
//
// enum Keys {
//
// KEY_UP = 1000,
//
// KEY_DOWN = 1001,
//
// KEY_LEFT = 1002,
//
// KEY_RIGHT = 1003,
//
// KEY_ENTER = 13,
//
// KEY_ESC = 27,
//
// KEY_BACKSPACE = 8
//
// };
//
