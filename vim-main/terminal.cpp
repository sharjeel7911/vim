#include "terminal.h"

Terminal::Terminal() {
  enableRawMode();
  getWindowSize();
  write(STDOUT_FILENO, "\x1b[?1049h\x1b[2J\x1b[H", 11);
}
Terminal::~Terminal() {
  write(STDOUT_FILENO, "\x1b[?1049l\x1b[?25h", 14);
  disableRawMode();
}

void Terminal::render(TextEditor &editor) {
  // Re-verify terminal dimensions in case user resized their window
  getWindowSize();
  AppendBuffer ab;

  // 1. Hide cursor while drawing to prevent visible "ghost" movements
  ab.append("\x1b[?25l");
  // Reposition terminal cursor to top-left corner
  ab.append("\x1b[H");

  size_t rowCount = editor.buffer.getRowCount();
  size_t offset = editor.getStatus().rowOffset;

  // Compute text viewport height (leave 2 rows at the bottom for status info)
  int textWindowHeight = screenRows - 2;

  // Calculate dynamic line number gutter width based on total rows
  int rowNumWidth = 2; // Default minimum width
  size_t tempRows = editor.status.totalRows;
  int digits = 0;
  while (tempRows > 0) {
    digits++;
    tempRows /= 10;
  }
  if (digits > rowNumWidth) {
    rowNumWidth = digits;
  }

  int maxLineLen = screenCols - (rowNumWidth + 2); // Printable space per line
  int visualRowsDrawn = 0;
  size_t fileRowIndex = offset;

  // Track cursor screen coordinates for the final placement
  int targetCursorRow = 1;
  int targetCursorCol = 1 + rowNumWidth + 2;

  // Render text lines up to the visible window height
  while (visualRowsDrawn < textWindowHeight) {

    if (fileRowIndex < rowCount) {
      string fullLine = editor.buffer.getRowText(fileRowIndex);
      if (!fullLine.empty() && fullLine.back() == '\n') {
        fullLine.pop_back();
      }

      // If the line is empty, render just the line number and a blank line
      if (fullLine.empty()) {
        stringstream ss;
        if (fileRowIndex + 1 < 10) {
          ss << setw(rowNumWidth) << setfill('0') << right << (fileRowIndex + 1)
             << "  ";
        } else {
          ss << setw(rowNumWidth) << setfill(' ') << right << (fileRowIndex + 1)
             << "  ";
        }
        ab.append(ss.str());

        // Track cursor if it is on this empty row
        if (fileRowIndex == editor.status.cursorRow - 1) {
          targetCursorRow = visualRowsDrawn + 1;
          targetCursorCol = rowNumWidth + 3;
        }

        ab.append("\x1b[K\r\n");
        visualRowsDrawn++;
      } else {
        // Line wrapping logic: chop the string into visual segments
        size_t startChar = 0;
        bool isFirstSegment = true;

        while (startChar < fullLine.length() &&
               visualRowsDrawn < textWindowHeight) {
          string segment = fullLine.substr(startChar, maxLineLen);

          stringstream ss;
          if (isFirstSegment) {
            // Print the actual line number only on the first segment
            if (fileRowIndex + 1 < 10) {
              ss << setw(rowNumWidth) << setfill('0') << right
                 << (fileRowIndex + 1) << "  ";
            } else {
              ss << setw(rowNumWidth) << setfill(' ') << right
                 << (fileRowIndex + 1) << "  ";
            }
            isFirstSegment = false;
          } else {
            // Wrapped lines get empty gutter padding so text aligns beautifully
            ss << string(rowNumWidth + 2, ' ');
          }

          ab.append(ss.str());
          ab.append(segment);
          ab.append("\x1b[K\r\n");

          // Calculate cursor positioning inside wrapped lines
          if (fileRowIndex == editor.status.cursorRow - 1) {
            size_t cursorInFileIdx = editor.status.cursorCol - 1;
            if (cursorInFileIdx >= startChar &&
                cursorInFileIdx <= startChar + segment.length()) {
              targetCursorRow = visualRowsDrawn + 1;
              targetCursorCol = (cursorInFileIdx - startChar) + rowNumWidth + 3;
            }
          }

          startChar += maxLineLen;
          visualRowsDrawn++;
        }
      }
    } else {
      // Past the end of file data: draw traditional vim tildes
      ab.append("~\x1b[K\r\n");
      visualRowsDrawn++;
    }

    fileRowIndex++;
  }

  // =================================================================
  // 2. RENDER FANCY STATUS INFORMATION BAR
  // =================================================================
  std::stringstream statusPos;
  statusPos << "\x1b[" << (screenRows - 1) << ";1H";
  ab.append(statusPos.str());

  string BG_DARK = "\x1b[48;5;235m";
  string TEXT_WHITE = "\x1b[38;5;255m";
  string TEXT_MUTED = "\x1b[38;5;244m";
  string RESET = "\x1b[0m";

  string modeStr = " NORMAL ";
  string modeColor = "\x1b[48;5;33m\x1b[38;5;232m\x1b[1m";

  if (editor.status.currMode == EditorMode::INSERT) {
    modeStr = " INSERT ";
    modeColor = "\x1b[48;5;71m\x1b[38;5;232m\x1b[1m";
  } else if (editor.status.currMode == EditorMode::COMMAND) {
    modeStr = " COMMAND ";
    modeColor = "\x1b[48;5;196m\x1b[38;5;255m\x1b[1m";
  }

  string modifiedIndicator = editor.status.unsavedChanges ? " ⏺ MODIFIED " : "";
  string modColor = editor.status.unsavedChanges ? "\x1b[38;5;208m" : "";
  string fileInfo = editor.fileManager.isFileOpen()
                        ? editor.fileManager.getCurrentFileName()
                        : "No File Open";

  std::stringstream leftSide, rightSide;
  leftSide << modeColor << modeStr << RESET << BG_DARK << TEXT_WHITE << "  "
           << fileInfo << modColor << modifiedIndicator;
  rightSide << TEXT_MUTED << "┃ " << TEXT_WHITE << "Ln "
            << editor.status.cursorRow << TEXT_MUTED << ", " << TEXT_WHITE
            << "Col " << editor.status.cursorCol << "  " << modeColor << " "
            << editor.status.totalRows << " Lns " << RESET;

  int visibleLeftLen =
      modeStr.length() + 2 + fileInfo.length() + modifiedIndicator.length();
  int visibleRightLen =
      2 + 3 + std::to_string(editor.status.cursorRow).length() + 2 + 4 +
      std::to_string(editor.status.cursorCol).length() + 2 +
      std::to_string(editor.status.totalRows).length() + 5;

  int middlePadding = screenCols - (visibleLeftLen + visibleRightLen);
  if (middlePadding < 0)
    middlePadding = 0;

  ab.append(leftSide.str());
  ab.append(BG_DARK + string(middlePadding, ' '));
  ab.append(rightSide.str());
  ab.append(RESET + "\x1b[K");

  // =================================================================
  // 3. RENDER COMMAND / NOTIFICATION BAR
  // =================================================================
  std::stringstream cmdPos;
  cmdPos << "\x1b[" << screenRows << ";1H\x1b[K";
  ab.append(cmdPos.str());

  if (editor.status.currMode == EditorMode::COMMAND) {
    ab.append("\x1b[38;5;214m\x1b[1m:\x1b[0m" + editor.status.lastCommand);
  } else if (!editor.status.statusMessage.empty()) {
    ab.append("\x1b[38;5;86m\x1b[3m " + editor.status.statusMessage +
              "\x1b[0m");
  }

  // =================================================================
  // 4. POSITION THE CURSOR AND SHOW IT AGAIN
  // =================================================================
  std::stringstream cursorMove;
  if (editor.status.currMode == EditorMode::COMMAND) {
    cursorMove << "\x1b[" << screenRows << ";"
               << (editor.status.lastCommand.length() + 2) << "H";
  } else {
    // Protect coordinates from rendering past visual window limits
    if (targetCursorRow > textWindowHeight)
      targetCursorRow = textWindowHeight;
    if (targetCursorRow < 1)
      targetCursorRow = 1;

    cursorMove << "\x1b[" << targetCursorRow << ";" << targetCursorCol << "H";
  }
  ab.append(cursorMove.str());
  ab.append("\x1b[?25h");

  write(STDOUT_FILENO, ab.buff.c_str(), ab.buff.length());
}

void Terminal::enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void Terminal::disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void Terminal::getWindowSize() {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_row == 0) {
    // Fallback defaults if ioctl fails
    screenRows = 24;
    screenCols = 80;
  } else {
    screenRows = ws.ws_row;
    screenCols = ws.ws_col;
  }
}

size_t Terminal::getScreenRows() { return screenRows; }
size_t Terminal::getScreenCols() { return screenCols; }

InputKey Terminal::readKey() {
  char c;
  if (read(STDIN_FILENO, &c, 1) != 1)
    return InputKey::UNKNOWN;

  // ESCAPE SEQUENCES
  if (c == '\x1b') {
    char seq[3];

    // Temporarily set read to non-blocking to look for trailing sequence bytes
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    struct termios original_raw = raw;
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1; // 100ms window
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    int r0 = read(STDIN_FILENO, &seq[0], 1);
    int r1 = read(STDIN_FILENO, &seq[1], 1);

    // Restore standard blocking properties immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &original_raw);

    if (r0 != 1 || r1 != 1)
      return InputKey::ESCAPE; // Tapped standalone ESC

    // CSI sequences processing
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        // Safe lookahead for trailing tilde sequence bytes
        tcgetattr(STDIN_FILENO, &raw);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        int r2 = read(STDIN_FILENO, &seq[2], 1);
        tcsetattr(STDIN_FILENO, TCSANOW, &original_raw);

        if (r2 != 1) {
          return InputKey::UNKNOWN;
        } else if (seq[2] == '~' && seq[1] == '3') {
          return InputKey::DELETE_KEY;
        }
      } else {
        switch (seq[1]) {
        case 'A':
          return InputKey::ARROW_UP;
        case 'B':
          return InputKey::ARROW_DOWN;
        case 'C':
          return InputKey::ARROW_RIGHT;
        case 'D':
          return InputKey::ARROW_LEFT;
        }
      }
    }
    return InputKey::ESCAPE;
  }

  // NORMAL KEYS
  switch (c) {
  case 127:
    return InputKey::BACKSPACE;
  case '\r':
  case '\n':
    return InputKey::ENTER;
  case '\t':
    return InputKey::TAB;
  }
  return (InputKey)c;
}
