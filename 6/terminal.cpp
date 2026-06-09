#include "terminal.h"

Terminal::Terminal() {
  enableRawMode();
  getWindowSize();
  // Using clean system writes
  string initSeq = AnsiSeq::altBufferOn() + AnsiSeq::clearScreen() +
                   AnsiSeq::moveCursor(1, 1);
  write(STDOUT_FILENO, initSeq.c_str(), initSeq.length());
}

Terminal::~Terminal() {
  string exitSeq = AnsiSeq::altBufferOff() + AnsiSeq::showCursor();
  write(STDOUT_FILENO, exitSeq.c_str(), exitSeq.length());
  disableRawMode();
}

// void Terminal::render(TextEditor &editor) {
//   getWindowSize();
//   AppendBuffer ab;

//   // Use our beautiful namespace wrappers!
//   ab.append(AnsiSeq::hideCursor());
//   ab.append(AnsiSeq::moveCursor(1, 1));

//   size_t rowCount = editor.buffer.getRowCount();
//   size_t offset = editor.getStatus().rowOffset;
//   int textWindowHeight = screenRows - 2;

//   int rowNumWidth = 2;
//   size_t tempRows = editor.status.totalRows;
//   int digits = 0;
//   while (tempRows > 0) {
//     digits++;
//     tempRows /= 10;
//   }
//   if (digits > rowNumWidth)
//     rowNumWidth = digits;

//   int maxLineLen = screenCols - (rowNumWidth + 2);
//   int visualRowsDrawn = 0;
//   size_t fileRowIndex = offset;

//   int targetCursorRow = 1;
//   int targetCursorCol = 1 + rowNumWidth + 2;

//   // Cache lower search pattern for case-insensitive screen highlighting
//   string rawPattern = editor.searchEngine.getLastSearchPattern();
//   string searchPattern = "";
//   for (char c : rawPattern)
//     searchPattern += std::tolower(c);

//   while (visualRowsDrawn < textWindowHeight) {
//     if (fileRowIndex < rowCount) {
//       string fullLine = editor.buffer.getRowText(fileRowIndex);
//       if (!fullLine.empty() && fullLine.back() == '\n') {
//         fullLine.pop_back();
//       }

//       // --- RENDER BLANK ROW ---
//       if (fullLine.empty()) {
//         stringstream ss;
//         // CHANGED: Always use space fill, never '0'
//         ss << setw(rowNumWidth) << setfill(' ') << right << (fileRowIndex +
//         1)
//            << "  ";
//         ab.append(ss.str());

//         if (fileRowIndex == editor.status.cursorRow - 1) {
//           targetCursorRow = visualRowsDrawn + 1;
//           targetCursorCol = rowNumWidth + 3;
//         }
//         ab.append(AnsiSeq::clearLine() + "\r\n");
//         visualRowsDrawn++;
//       }
//       // --- RENDER LINE WITH HIGHLIGHT EXTRACTION ---
//       else {
//         size_t startChar = 0;
//         bool isFirstSegment = true;

//         while (startChar < fullLine.length() &&
//                visualRowsDrawn < textWindowHeight) {
//           string segment = fullLine.substr(startChar, maxLineLen);

//           stringstream ss;
//           if (isFirstSegment) {
//             // CHANGED: Always use space fill, never '0'
//             ss << setw(rowNumWidth) << setfill(' ') << right
//                << (fileRowIndex + 1) << "  ";
//             isFirstSegment = false;
//           } else {
//             ss << string(rowNumWidth + 2, ' ');
//           }
//           ab.append(ss.str());

//           // Apply Search Highlighting to the visible segment string
//           if (!searchPattern.empty()) {
//             string lowerSegment = "";
//             for (char c : segment)
//               lowerSegment += std::tolower(c);

//             size_t matchPos = lowerSegment.find(searchPattern);
//             size_t segCursor = 0;

//             while (matchPos != string::npos) {
//               // Append text before match
//               ab.append(segment.substr(segCursor, matchPos - segCursor));
//               // Append Highlighted match
//               ab.append(AnsiSeq::searchHighlight() +
//                         segment.substr(matchPos, searchPattern.length()) +
//                         AnsiSeq::reset());

//               segCursor = matchPos + searchPattern.length();
//               matchPos = lowerSegment.find(searchPattern, segCursor);
//             }
//             // Append remaining segment string text
//             ab.append(segment.substr(segCursor));
//           } else {
//             ab.append(segment); // No active search pattern
//           }

//           ab.append(AnsiSeq::clearLine() + "\r\n");

//           // Track visual coordinates
//           if (fileRowIndex == editor.status.cursorRow - 1) {
//             size_t cursorInFileIdx = editor.status.cursorCol - 1;
//             if (cursorInFileIdx >= startChar &&
//                 cursorInFileIdx <= startChar + segment.length()) {
//               targetCursorRow = visualRowsDrawn + 1;
//               targetCursorCol = (cursorInFileIdx - startChar) + rowNumWidth +
//               3;
//             }
//           }
//           startChar += maxLineLen;
//           visualRowsDrawn++;
//         }
//       }
//     } else {
//       ab.append("~" + AnsiSeq::clearLine() + "\r\n");
//       visualRowsDrawn++;
//     }
//     fileRowIndex++;
//   }

//   // --- STATUS BAR RENDERING ---
//   ab.append(AnsiSeq::moveCursor(screenRows - 1, 1));
//   string BG_DARK = "\x1b[48;5;235m";
//   string TEXT_WHITE = "\x1b[38;5;255m";
//   string TEXT_MUTED = "\x1b[38;5;244m";
//   string RESET = AnsiSeq::reset();

//   string modeStr = " NORMAL ";
//   string modeColor = "\x1b[48;5;33m\x1b[38;5;232m\x1b[1m";

//   if (editor.status.currMode == EditorMode::INSERT) {
//     modeStr = " INSERT ";
//     modeColor = "\x1b[48;5;71m\x1b[38;5;232m\x1b[1m";
//   } else if (editor.status.currMode == EditorMode::COMMAND) {
//     modeStr = " COMMAND ";
//     modeColor = "\x1b[48;5;196m\x1b[38;5;255m\x1b[1m";
//   }

//   string modifiedIndicator = editor.status.unsavedChanges ? " ⏺ MODIFIED " :
//   ""; string modColor = editor.status.unsavedChanges ? "\x1b[38;5;208m" : "";
//   string fileInfo = editor.fileManager.isFileOpen()
//                         ? editor.fileManager.getCurrentFileName()
//                         : "No File Open";

//   std::stringstream leftSide, rightSide;
//   leftSide << modeColor << modeStr << RESET << BG_DARK << TEXT_WHITE << "  "
//            << fileInfo << modColor << modifiedIndicator;
//   rightSide << TEXT_MUTED << "┃ " << TEXT_WHITE << "Ln "
//             << editor.status.cursorRow << TEXT_MUTED << ", " << TEXT_WHITE
//             << "Col " << editor.status.cursorCol << "  " << modeColor << " "
//             << editor.status.totalRows << " Lns " << RESET;

//   int visibleLeftLen =
//       modeStr.length() + 2 + fileInfo.length() + modifiedIndicator.length();
//   int visibleRightLen =
//       2 + 3 + std::to_string(editor.status.cursorRow).length() + 2 + 4 +
//       std::to_string(editor.status.cursorCol).length() + 2 +
//       std::to_string(editor.status.totalRows).length() + 5;
//   int middlePadding = screenCols - (visibleLeftLen + visibleRightLen);
//   if (middlePadding < 0)
//     middlePadding = 0;

//   ab.append(leftSide.str() + BG_DARK + string(middlePadding, ' ') +
//             rightSide.str() + RESET + AnsiSeq::clearLine());

//   // --- COMMAND / NOTIFICATION ROW ---
//   ab.append(AnsiSeq::moveCursor(screenRows, 1) + AnsiSeq::clearLine());
//   if (editor.status.currMode == EditorMode::COMMAND) {
//     ab.append("\x1b[38;5;214m\x1b[1m:\x1b[0m" + editor.status.lastCommand);
//   } else if (!editor.status.statusMessage.empty()) {
//     ab.append("\x1b[38;5;86m\x1b[3m " + editor.status.statusMessage + RESET);
//   }

//   // --- EXECUTE FINAL CURSOR POSITION ---
//   if (editor.status.currMode == EditorMode::COMMAND) {
//     ab.append(AnsiSeq::moveCursor(screenRows,
//                                   editor.status.lastCommand.length() + 2));
//   } else {
//     if (targetCursorRow > textWindowHeight)
//       targetCursorRow = textWindowHeight;
//     if (targetCursorRow < 1)
//       targetCursorRow = 1;
//     ab.append(AnsiSeq::moveCursor(targetCursorRow, targetCursorCol));
//   }
//   ab.append(AnsiSeq::showCursor());

//   write(STDOUT_FILENO, ab.buff.c_str(), ab.buff.length());
// }

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

  if (c == '\x1b') {
    char seq[3];
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    struct termios original_raw = raw;
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    int r0 = read(STDIN_FILENO, &seq[0], 1);
    int r1 = read(STDIN_FILENO, &seq[1], 1);

    tcsetattr(STDIN_FILENO, TCSANOW, &original_raw);

    if (r0 != 1 || r1 != 1)
      return InputKey::ESCAPE;

    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        tcgetattr(STDIN_FILENO, &raw);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        int r2 = read(STDIN_FILENO, &seq[2], 1);
        tcsetattr(STDIN_FILENO, TCSANOW, &original_raw);

        if (r2 != 1)
          return InputKey::UNKNOWN;
        if (seq[2] == '~' && seq[1] == '3')
          return InputKey::DELETE_KEY;
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

void Terminal::render(TextEditor &editor) {
  getWindowSize();
  AppendBuffer ab;

  ab.append(AnsiSeq::hideCursor());
  ab.append(AnsiSeq::moveCursor(1, 1));

  size_t rowCount = editor.buffer.getRowCount();
  size_t offset = editor.getStatus().rowOffset;
  int textWindowHeight = screenRows - 2;

  // Calculate clean dynamic margin for line numbers
  int rowNumWidth = 2;
  size_t tempRows = editor.status.totalRows;
  int digits = 0;
  while (tempRows > 0) {
    digits++;
    tempRows /= 10;
  }
  if (digits > rowNumWidth)
    rowNumWidth = digits;

  int maxLineLen = screenCols - (rowNumWidth + 2);
  int visualRowsDrawn = 0;
  size_t fileRowIndex = offset;

  // Track exact screen coordinates
  int targetCursorRow = 1;
  int targetCursorCol = rowNumWidth + 3;
  bool cursorPlaced = false;

  string rawPattern = editor.searchEngine.getLastSearchPattern();
  string searchPattern = "";
  for (char c : rawPattern)
    searchPattern += std::tolower(c);

  while (visualRowsDrawn < textWindowHeight) {
    if (fileRowIndex < rowCount) {
      string fullLine = editor.buffer.getRowText(fileRowIndex);
      if (!fullLine.empty() && fullLine.back() == '\n') {
        fullLine.pop_back();
      }

      bool isCursorRow = (fileRowIndex == editor.status.cursorRow - 1);

      if (fullLine.empty()) {
        stringstream ss;
        ss << setw(rowNumWidth) << setfill(' ') << right << (fileRowIndex + 1)
           << "  ";
        ab.append(ss.str());

        if (isCursorRow) {
          targetCursorRow = visualRowsDrawn + 1;
          targetCursorCol = rowNumWidth + 3;
          cursorPlaced = true;
        }
        ab.append(AnsiSeq::clearLine() + "\r\n");
        visualRowsDrawn++;
      } else {
        size_t startChar = 0;
        bool isFirstSegment = true;

        while (startChar < fullLine.length() &&
               visualRowsDrawn < textWindowHeight) {
          string segment = fullLine.substr(startChar, maxLineLen);

          stringstream ss;
          if (isFirstSegment) {
            ss << setw(rowNumWidth) << setfill(' ') << right
               << (fileRowIndex + 1) << "  ";
            isFirstSegment = false;
          } else {
            ss << string(rowNumWidth + 2, ' ');
          }
          ab.append(ss.str());

          if (!searchPattern.empty()) {
            string lowerSegment = "";
            for (char c : segment)
              lowerSegment += std::tolower(c);
            size_t matchPos = lowerSegment.find(searchPattern);
            size_t segCursor = 0;

            while (matchPos != string::npos) {
              ab.append(segment.substr(segCursor, matchPos - segCursor));
              ab.append(AnsiSeq::searchHighlight() +
                        segment.substr(matchPos, searchPattern.length()) +
                        AnsiSeq::reset());
              segCursor = matchPos + searchPattern.length();
              matchPos = lowerSegment.find(searchPattern, segCursor);
            }
            ab.append(segment.substr(segCursor));
          } else {
            ab.append(segment);
          }

          ab.append(AnsiSeq::clearLine() + "\r\n");

          if (isCursorRow) {
            size_t cursorInFileIdx = editor.status.cursorCol - 1;
            if (cursorInFileIdx >= startChar &&
                cursorInFileIdx <= startChar + segment.length()) {
              targetCursorRow = visualRowsDrawn + 1;
              targetCursorCol = (cursorInFileIdx - startChar) + rowNumWidth + 3;
              cursorPlaced = true;
            }
          }
          startChar += maxLineLen;
          visualRowsDrawn++;
        }
      }
    } else {
      ab.append("~" + AnsiSeq::clearLine() + "\r\n");
      visualRowsDrawn++;
    }
    fileRowIndex++;
  }

  // --- STATUS BAR RENDERING ---
  ab.append(AnsiSeq::moveCursor(screenRows - 1, 1));
  string BG_DARK = "\x1b[48;5;235m", TEXT_WHITE = "\x1b[38;5;255m",
         TEXT_MUTED = "\x1b[38;5;244m", RESET = AnsiSeq::reset();
  string modeStr = (editor.status.currMode == EditorMode::INSERT) ? " INSERT"
                   : (editor.status.currMode == EditorMode::COMMAND)
                       ? " COMMAND "
                       : " NORMAL ";
  string modeColor = (editor.status.currMode == EditorMode::INSERT)
                         ? "\x1b[48;5;71m\x1b[38;5;232m\x1b[1m"
                     : (editor.status.currMode == EditorMode::COMMAND)
                         ? "\x1b[48;5;196m\x1b[38;5;255m\x1b[1m"
                         : "\x1b[48;5;33m\x1b[38;5;232m\x1b[1m";

  string modifiedIndicator = editor.status.unsavedChanges ? " ⏺ MODIFIED " : "";
  string modColor = editor.status.unsavedChanges ? "\x1b[38;5;208m" : "";
  string fileInfo = editor.fileManager.isFileOpen()
                        ? editor.fileManager.getCurrentFileName()
                        : "No File Open";

  stringstream leftSide, rightSide;
  leftSide << modeColor << modeStr << RESET << BG_DARK << TEXT_WHITE << "  "
           << fileInfo << modColor << modifiedIndicator;
  rightSide << TEXT_MUTED << "┃ " << TEXT_WHITE << "Ln "
            << editor.status.cursorRow << TEXT_MUTED << ", " << TEXT_WHITE
            << "Col " << editor.status.cursorCol << "  " << modeColor << " "
            << editor.status.totalRows << " Lns " << RESET;

  int visibleLeftLen =
      modeStr.length() + 2 + fileInfo.length() + modifiedIndicator.length();
  int visibleRightLen = 2 + 3 + to_string(editor.status.cursorRow).length() +
                        2 + 4 + to_string(editor.status.cursorCol).length() +
                        2 + to_string(editor.status.totalRows).length() + 5;
  int middlePadding = screenCols - (visibleLeftLen + visibleRightLen);
  if (middlePadding < 0)
    middlePadding = 0;

  ab.append(leftSide.str() + BG_DARK + string(middlePadding, ' ') +
            rightSide.str() + RESET + AnsiSeq::clearLine());

  // --- COMMAND ROW SETUP ---
  ab.append(AnsiSeq::moveCursor(screenRows, 1) + AnsiSeq::clearLine());
  if (editor.status.currMode == EditorMode::COMMAND) {
    ab.append("\x1b[38;5;214m\x1b[1m:\x1b[0m" + editor.status.lastCommand);
    ab.append(AnsiSeq::moveCursor(screenRows,
                                  editor.status.lastCommand.length() + 2));
  } else {
    if (!editor.status.statusMessage.empty())
      ab.append("\x1b[38;5;86m\x1b[3m " + editor.status.statusMessage + RESET);

    // Ensure safety controls over active window boundary tracking
    if (!cursorPlaced) {
      targetCursorRow = textWindowHeight;
    }
    if (targetCursorRow > textWindowHeight)
      targetCursorRow = textWindowHeight;
    if (targetCursorRow < 1)
      targetCursorRow = 1;

    ab.append(AnsiSeq::moveCursor(targetCursorRow, targetCursorCol));
  }
  ab.append(AnsiSeq::showCursor());
  write(STDOUT_FILENO, ab.buff.c_str(), ab.buff.length());
}