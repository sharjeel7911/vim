#include "terminal.h"

Terminal::Terminal() {
	enableRawMode();
	getWindowSize();
	string initSeq = AnsiSeq::altBufferOn() + AnsiSeq::clearScreen() + AnsiSeq::moveCursor(1, 1);
	write(STDOUT_FILENO, initSeq.c_str(), initSeq.length());
}

Terminal::~Terminal() {
	string exitSeq = AnsiSeq::altBufferOff() + AnsiSeq::showCursor();
	write(STDOUT_FILENO, exitSeq.c_str(), exitSeq.length());
	disableRawMode();
}

void Terminal::enableRawMode() {
	tcgetattr(STDIN_FILENO, &orig_termios);
	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void Terminal::disableRawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

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

// added helper — expands '\t' to spaces up to next tab-stop so visual screen columns match what we compute for cursor placement.
string Terminal::expandTabsStr(const string& in, int tabStop) {
	string out;
	int col = 0;
	for (char c : in) {
		if (c == '\t') {
			int spaces = tabStop - (col % tabStop);
			out.append(spaces, ' ');
			col += spaces;
		} else {
			out += c;
			col++;
		}
	}
	return out;
}

InputKey Terminal::readKey() {
	char c;
	if (read(STDIN_FILENO, &c, 1) != 1) return InputKey::UNKNOWN;

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

		if (r0 != 1 || r1 != 1) return InputKey::ESCAPE;

		if (seq[0] == '[') {
			if (seq[1] >= '0' && seq[1] <= '9') {
				tcgetattr(STDIN_FILENO, &raw);
				raw.c_cc[VMIN] = 0;
				raw.c_cc[VTIME] = 1;
				tcsetattr(STDIN_FILENO, TCSANOW, &raw);
				int r2 = read(STDIN_FILENO, &seq[2], 1);
				tcsetattr(STDIN_FILENO, TCSANOW, &original_raw);

				if (r2 != 1) return InputKey::UNKNOWN;
				if (seq[2] == '~' && seq[1] == '3') return InputKey::DELETE_KEY;
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

void Terminal::render(TextEditor& editor) {
	getWindowSize();
	AppendBuffer ab;

	ab.append(AnsiSeq::hideCursor());
	ab.append(AnsiSeq::moveCursor(1, 1));

	size_t rowCount = editor.buffer.getRowCount();
	size_t offset = editor.getStatus().rowOffset;
	int textWindowHeight = screenRows - 2;

	// calculate dynamic margin for line numbers
	int rowNumWidth = 2;
	size_t tempRows = editor.status.totalRows;
	int digits = 0;
	while (tempRows > 0) {
		digits++;
		tempRows /= 10;
	}
	if (digits > rowNumWidth) {
		rowNumWidth = digits;
	}

	int maxLineLen = screenCols - (rowNumWidth + 2);
	int visualRowsDrawn = 0;
	size_t fileRowIndex = offset;

	// track exact screen coordinates
	int targetCursorRow = 1;
	int targetCursorCol = rowNumWidth + 3;
	bool cursorPlaced = false;

	string rawPattern = editor.searchEngine.getLastSearchPattern();
	string searchPattern = "";
	for (char c : rawPattern) {
		searchPattern += tolower(c);
	}

	while (visualRowsDrawn < textWindowHeight) {
		if (fileRowIndex < rowCount) {
			// keep the raw line (for mapping cursorCol -> visual col) and render from a tab-expanded copy so wrapping/columns are correct.
			string rawFullLine = editor.buffer.getRowText(fileRowIndex);
			if (!rawFullLine.empty() && rawFullLine.back() == '\n') {
				rawFullLine.pop_back();
			}
			string fullLine = expandTabsStr(rawFullLine);

			bool isCursorRow = (fileRowIndex == editor.status.cursorRow - 1);

			if (fullLine.empty()) {
				stringstream ss;
				ss << setw(rowNumWidth) << setfill(' ') << right << (fileRowIndex + 1) << "  ";
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

				while (startChar < fullLine.length() && visualRowsDrawn < textWindowHeight) {
					string segment = fullLine.substr(startChar, maxLineLen);

					stringstream ss;
					if (isFirstSegment) {
						ss << setw(rowNumWidth) << setfill(' ') << right << (fileRowIndex + 1) << "  ";
						isFirstSegment = false;
					} else {
						ss << string(rowNumWidth + 2, ' ');
					}

					ab.append(ss.str());

					if (!searchPattern.empty()) {
						string lowerSegment = "";
						for (char c : segment) {
							lowerSegment += tolower(c);
						}
						size_t matchPos = lowerSegment.find(searchPattern);
						size_t segCursor = 0;

						while (matchPos != string::npos) {
							ab.append(segment.substr(segCursor, matchPos - segCursor));
							ab.append(AnsiSeq::searchHighlight() + segment.substr(matchPos, searchPattern.length()) + AnsiSeq::reset());
							segCursor = matchPos + searchPattern.length();
							matchPos = lowerSegment.find(searchPattern, segCursor);
						}
						ab.append(segment.substr(segCursor));
					} else {
						ab.append(segment);
					}

					ab.append(AnsiSeq::clearLine() + "\r\n");

					if (isCursorRow) {
						// map raw (char-count) cursor column to its expanded visual column, since a tab before the cursor eats extra columns.
						size_t rawCursorCol = editor.status.cursorCol - 1;
						string rawPrefix = (rawCursorCol <= rawFullLine.length()) ? rawFullLine.substr(0, rawCursorCol) : rawFullLine;
						size_t cursorInFileIdx = expandTabsStr(rawPrefix).length();
						if (cursorInFileIdx >= startChar && cursorInFileIdx <= startChar + segment.length()) {
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
	string BG_DARK = "\x1b[48;5;235m", TEXT_WHITE = "\x1b[38;5;255m", TEXT_MUTED = "\x1b[38;5;244m", RESET = AnsiSeq::reset();
	string modeStr = (editor.status.currMode == EditorMode::INSERT) ? " INSERT" : (editor.status.currMode == EditorMode::COMMAND) ? " COMMAND " : " NORMAL ";
	string modeColor = (editor.status.currMode == EditorMode::INSERT) ? "\x1b[48;5;71m\x1b[38;5;232m\x1b[1m" : (editor.status.currMode == EditorMode::COMMAND) ? "\x1b[48;5;196m\x1b[38;5;255m\x1b[1m" : "\x1b[48;5;33m\x1b[38;5;232m\x1b[1m";

	string modifiedIndicator = editor.status.unsavedChanges ? " ⏺ MODIFIED " : "";
	string modColor = editor.status.unsavedChanges ? "\x1b[38;5;208m" : "";
	string fileInfo = editor.fileManager.isFileOpen() ? editor.fileManager.getCurrentFileName() : "No File Open";

	stringstream leftSide, rightSide;
	leftSide << modeColor << modeStr << RESET << BG_DARK << TEXT_WHITE << "  " << fileInfo << modColor << modifiedIndicator;
	// CHANGED: show the visual (tab-expanded) column here too, so the status
	// bar number matches the actual on-screen cursor position.
	string curRowRaw = editor.buffer.getRowText(editor.status.cursorRow - 1);
	if (!curRowRaw.empty() && curRowRaw.back() == '\n') curRowRaw.pop_back();
	size_t rawColIdx = (editor.status.cursorCol - 1 <= curRowRaw.length()) ? editor.status.cursorCol - 1 : curRowRaw.length();
	size_t displayCol = expandTabsStr(curRowRaw.substr(0, rawColIdx)).length() + 1;

	rightSide << TEXT_MUTED << "┃ " << TEXT_WHITE << "Ln " << editor.status.cursorRow << TEXT_MUTED << ", " << TEXT_WHITE << "Col " << displayCol << "  " << modeColor << " " << editor.status.totalRows << " Lns " << RESET;

	int visibleLeftLen = modeStr.length() + 2 + fileInfo.length() + modifiedIndicator.length();
	int visibleRightLen = 2 + 3 + to_string(editor.status.cursorRow).length() + 2 + 4 + to_string(displayCol).length() + 2 + to_string(editor.status.totalRows).length() + 5;
	to_string(editor.status.cursorCol).length() + 2 + to_string(editor.status.totalRows).length() + 5;
	int middlePadding = screenCols - (visibleLeftLen + visibleRightLen);
	if (middlePadding < 0) middlePadding = 0;

	ab.append(leftSide.str() + BG_DARK + string(middlePadding, ' ') + rightSide.str() + RESET + AnsiSeq::clearLine());

	// --- COMMAND ROW SETUP ---
	ab.append(AnsiSeq::moveCursor(screenRows, 1) + AnsiSeq::clearLine());
	if (editor.status.currMode == EditorMode::COMMAND) {
		ab.append("\x1b[38;5;214m\x1b[1m:\x1b[0m" + editor.status.lastCommand);
		ab.append(AnsiSeq::moveCursor(screenRows, editor.status.lastCommand.length() + 2));
	} else {
		if (!editor.status.statusMessage.empty()) {
			ab.append("\x1b[38;5;86m\x1b[3m " + editor.status.statusMessage + RESET);
		}
		if (!cursorPlaced) {
			targetCursorRow = textWindowHeight;
		}
		if (targetCursorRow > textWindowHeight) {
			targetCursorRow = textWindowHeight;
		}
		if (targetCursorRow < 1) {
			targetCursorRow = 1;
		}

		ab.append(AnsiSeq::moveCursor(targetCursorRow, targetCursorCol));
	}
	ab.append(AnsiSeq::showCursor());
	write(STDOUT_FILENO, ab.buff.c_str(), ab.buff.length());
}
