#include "gapbuffer.h"

void GapBuffer::regrowBuffer(size_t minSize) {
  size_t gapSize = getGapSize();
  if (gapSize >= minSize)
    return;

  size_t newCap = std::max(bufferSize * 2, bufferSize + minSize);
  char *newBuffer = new char[newCap];

  // copy left side
  for (size_t i = 0; i < gapStart; i++) {
    newBuffer[i] = buffer[i];
  }

  size_t rightSize = bufferSize - gapEnd;
  size_t newGapEnd = newCap - rightSize;

  // copy right side
  for (size_t i = 0; i < rightSize; i++) {
    newBuffer[newGapEnd + i] = buffer[gapEnd + i];
  }

  delete[] buffer;
  buffer = newBuffer;
  gapEnd = newGapEnd;
  bufferSize = newCap;
}

void GapBuffer::moveGapTo(size_t pos) {
  // move left
  if (pos < gapStart) {
    size_t shift = gapStart - pos;

    for (size_t i = 0; i < shift; i++) {
      buffer[gapEnd - 1 - i] = buffer[gapStart - 1 - i];
    }

    gapStart -= shift;
    gapEnd -= shift;
  }
  // move right
  else if (pos > gapStart) {
    size_t shift = pos - gapStart;

    for (size_t i = 0; i < shift; i++) {
      buffer[gapStart + i] = buffer[gapEnd + i];
    }

    gapStart += shift;
    gapEnd += shift;
  }
}

void GapBuffer::display() {
  for (size_t i = 0; i < gapStart; i++) {
    cout << buffer[i] << " ";
  }
  cout << endl;
  for (size_t i = gapEnd; i < bufferSize; i++) {
    cout << buffer[i] << " ";
  }
  cout << "\nGap: [" << gapStart << "," << gapEnd << "]\n";
  cout << endl;
}

// +--------------------------------------------------------------------------------------------------+
// Lifecycle
// +--------------------------------------------------------------------------------------------------+

GapBuffer::GapBuffer(size_t initialCap)
    : bufferSize(initialCap ? initialCap : 8), gapStart(0),
      gapEnd(initialCap ? initialCap : 8) {
  buffer = new char[bufferSize];
}

GapBuffer::~GapBuffer() { delete[] buffer; }

// +--------------------------------------------------------------------------------------------------+
// Editing
// +--------------------------------------------------------------------------------------------------+

void GapBuffer::insertCharAtCursor(char ch) {
  if (isGapFilled())
    regrowBuffer(1);
  buffer[gapStart++] = ch;
}

void GapBuffer::insertStringAtCursor(const string &str) {
  size_t len = str.length();

  if (getGapSize() < len) {
    regrowBuffer(len);
  }

  for (size_t i = 0; i < len; i++) {
    buffer[gapStart++] = str[i];
  }
}

void GapBuffer::insertStringAtPos(size_t pos, const string &str) {
  size_t limit = getTextLength();

  if (pos > limit) {
    pos = limit;
  }
  moveGapTo(pos); // jump to the target
  insertStringAtCursor(str);
}

void GapBuffer::deleteCharBeforeCursor() {
  if (gapStart > 0) {
    deleteTextInRange(gapStart - 1, gapStart);
  }
}

void GapBuffer::deleteCharAfterCursor() {
  if (gapStart < getTextLength()) {
    deleteTextInRange(gapStart, gapStart + 1);
  }
}

void GapBuffer::deleteTextInRange(size_t start, size_t end) {
  if (start >= end)
    return;
  moveGapTo(start);
  size_t count = end - start;
  if ((gapEnd + count) > bufferSize)
    return;

  gapEnd += count;
}

void GapBuffer::replaceTextInRange(size_t start, size_t end,
                                   const char *replacement) {
  deleteTextInRange(start, end);
  insertStringAtPos(start, replacement);
}

void GapBuffer::clearBuffer() {
  gapStart = 0;
  gapEnd = bufferSize;
}

// +--------------------------------------------------------------------------------------------------+
// Cursor movement
// +--------------------------------------------------------------------------------------------------+

void GapBuffer::moveCursorLeft() {
  if (gapStart > 0) {
    moveGapTo(gapStart - 1);
  }
}

void GapBuffer::moveCursorRight() {
  if (gapStart < getTextLength()) {
    moveGapTo(gapStart + 1);
  }
}

void GapBuffer::moveCursorUp(size_t currentCol) {
  size_t currentRow = getRowIndexFromPos(gapStart);
  if (currentRow == 0)
    return;

  size_t prevLineStart = getRowStartIndex(currentRow - 1);
  size_t prevLineEnd = getRowEndIndex(currentRow - 1);

  size_t targetPos = prevLineStart + currentCol;
  if (targetPos > prevLineEnd)
    targetPos = prevLineEnd;
  moveGapTo(targetPos);
}

void GapBuffer::moveCursorDown(size_t currentCol) {
  size_t currentRow = getRowIndexFromPos(gapStart);
  if (currentRow >= getRowCount() - 1)
    return;

  size_t nextLineStart = getRowStartIndex(currentRow + 1);
  size_t nextLineEnd = getRowEndIndex(currentRow + 1);

  size_t targetPos = nextLineStart + currentCol;
  if (targetPos > nextLineEnd)
    targetPos = nextLineEnd;
  moveGapTo(targetPos);
}

void GapBuffer::moveCursorToPos(size_t pos) {
  size_t textSize = getTextLength();
  if (pos > textSize)
    pos = textSize;

  moveGapTo(pos);
}

void GapBuffer::moveCursorToSpecificRow(size_t rowNum) {
  moveGapTo(getRowStartIndex(rowNum));
}

void GapBuffer::moveCursorToRowStart() {
  size_t pos = gapStart;
  while (pos > 0 && getCharAt(pos - 1) != '\n') {
    pos--;
  }
  moveGapTo(pos);
}

void GapBuffer::moveCursorToRowEnd() {
  size_t pos = gapStart;
  while (pos < getTextLength() && getCharAt(pos) != '\n') {
    pos++;
  }
  moveGapTo(pos);
}

void GapBuffer::moveCursorByWord(bool forward) {
  if (forward) {
    // if currently on a word, skip to the end of it
    if (gapStart < getTextLength() && isWordChar(getCharAt(gapStart))) {
      while (gapStart < getTextLength() && isWordChar(getCharAt(gapStart))) {
        moveCursorRight();
      }
    }
    // if on punctuation, skip it
    else if (gapStart < getTextLength() && !isspace(getCharAt(gapStart))) {
      moveCursorRight();
    }
    // skip whitespace
    while (gapStart < getTextLength() && isspace(getCharAt(gapStart))) {
      moveCursorRight();
    }
  } else {
    // backward logic (look at gapStart - 1)
    // skip whitespace
    while (gapStart > 0 && isspace(getCharAt(gapStart - 1))) {
      moveCursorLeft();
    }
    // if on word, skip to start
    if (gapStart > 0 && isWordChar(getCharAt(gapStart - 1))) {
      while (gapStart > 0 && isWordChar(getCharAt(gapStart - 1)))
        moveCursorLeft();
    }
    // if on punctuation, skip it
    else if (gapStart > 0 && !isspace(getCharAt(gapStart - 1))) {
      moveCursorLeft();
    }
  }
}

void GapBuffer::moveCursorToEndOfWord() {
  // skip leading spaces
  while (gapStart < getTextLength() && isspace(getCharAt(gapStart))) {
    moveCursorRight();
  }
  // move to the last char of the word
  while (gapStart < getTextLength() - 1 &&
         isWordChar(getCharAt(gapStart + 1))) {
    moveCursorRight();
  }
}

void GapBuffer::moveCursorToBufferStart() { moveGapTo(0); }
void GapBuffer::moveCursorToBufferEnd() { moveGapTo(getTextLength()); }

// +--------------------------------------------------------------------------------------------------+
// Text access
// +--------------------------------------------------------------------------------------------------+

char GapBuffer::getCharAt(size_t pos) {
  if (pos >= getTextLength())
    return '\0';

  // position is before gap
  if (pos < gapStart) {
    return buffer[pos];
  }
  // position is after gap
  else {
    return buffer[gapEnd + (pos - gapStart)];
  }
}

string GapBuffer::getRowText(size_t rowNum) {
  size_t start = getRowStartIndex(rowNum);
  size_t end = getRowEndIndex(rowNum);

  string row = "";
  for (size_t i = start; i < end; i++) {
    row += getCharAt(i);
  }
  if (end < getTextLength() && getCharAt(end) == '\n') {
    row += '\n';
  }
  return row;
}

string GapBuffer::getWholeBufferText() {
  string text = "";
  for (size_t i = 0; i < getTextLength(); i++) {
    text += getCharAt(i);
  }
  return text;
}

string GapBuffer::getTextInRange(size_t start, size_t end) {
  string str = "";
  for (size_t i = start; i < end; i++) {
    str += getCharAt(i);
  }
  return str;
}

// +--------------------------------------------------------------------------------------------------+
// State queries
// +--------------------------------------------------------------------------------------------------+
bool GapBuffer::isWordChar(char c) {
  return (isalnum(c) || c == '_' || c == '(' || c == ')' || c == '[' ||
          c == ']');
}
char &GapBuffer::operator[](size_t index) { return buffer[index]; }
size_t GapBuffer::getGapStart() { return gapStart; }
bool GapBuffer::isBufferEmpty() { return getTextLength() == 0; }
bool GapBuffer::isGapFilled() { return getGapSize() == 0; }
size_t GapBuffer::getBufferCapacity() { return bufferSize; }
size_t GapBuffer::getTextLength() { return bufferSize - getGapSize(); }
size_t GapBuffer::getGapSize() { return gapEnd - gapStart; }
size_t GapBuffer::getCursorPosition() { return gapStart; }

size_t GapBuffer::getRowCount() {
  size_t count = 1;
  for (size_t i = 0; i < getTextLength(); i++) {
    if (getCharAt(i) == '\n')
      count++;
  }
  return count;
}

size_t GapBuffer::getRowIndexFromPos(size_t pos) {
  size_t row = 0;
  for (size_t i = 0; i < pos && i < getTextLength(); i++) {
    if (getCharAt(i) == '\n')
      row++;
  }
  return row;
}

size_t GapBuffer::getColIndexFromPos(size_t pos) {
  size_t start = getRowStartIndex(getRowIndexFromPos(pos));
  return pos - start;
}

size_t GapBuffer::getCurrentRowIndex() {
  return getRowIndexFromPos(gapStart) + 1;
}
size_t GapBuffer::getCurrentColIndex() {
  return getColIndexFromPos(gapStart) + 1;
}

size_t GapBuffer::getRowStartIndex(size_t rowIndex) {
  if (rowIndex == 0)
    return 0;

  size_t count = 0;
  for (size_t i = 0; i < getTextLength(); i++) {
    if (getCharAt(i) == '\n') {
      count++;
      if (count == rowIndex)
        return i + 1; // next char after '\n' is start of line so returns index
                      // of first char of current line
    }
  }
  return getTextLength();
}

size_t GapBuffer::getRowEndIndex(size_t rowIndex) {
  size_t start = getRowStartIndex(rowIndex);
  for (size_t i = start; i < getTextLength(); i++) {
    if (getCharAt(i) == '\n')
      return i; // returns index of '\n' which is end of line
  }
  return getTextLength();
}

size_t GapBuffer::findNextPatternFirstIndex(const char *pattern, size_t start) {
  size_t patternLen = strlen(pattern);
  size_t textLen = getTextLength();
  if (patternLen == 0 || start + patternLen > textLen)
    return (size_t)-1;

  for (size_t i = start; i <= textLen - patternLen; ++i) {
    bool match = true;
    for (size_t j = 0; j < patternLen; ++j) {
      if (getCharAt(i + j) != pattern[j]) {
        match = false;
        break;
      }
    }
    if (match)
      return i;
  }
  return (size_t)-1;
}

size_t GapBuffer::findPreviousPatternFirstIndex(const char *pattern,
                                                size_t start) {
  size_t patternLen = strlen(pattern);
  if (patternLen == 0 || start < patternLen)
    return (size_t)-1;

  for (size_t i = start - patternLen; i != (size_t)-1; --i) {
    bool match = true;
    for (size_t j = 0; j < patternLen; ++j) {
      if (getCharAt(i + j) != pattern[j]) {
        match = false;
        break;
      }
    }
    if (match)
      return i;
  }
  return (size_t)-1;
}
