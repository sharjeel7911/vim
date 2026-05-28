#include "editor.h"
#include <cstddef>

TextEditor::TextEditor() {
  status.currMode = EditorMode::NORMAL;
  status.lastCommand = "";
  status.statusMessage = "";
  status.cursorRow = 1;
  status.cursorCol = 1;
  status.unsavedChanges = false;
  updateStatus(22);
}

EditorStatus &TextEditor::getStatus() { return status; }
FileManager &TextEditor::getFileManager() { return fileManager; }
GapBuffer &TextEditor::getBuffer() { return buffer; }

// void TextEditor::run(const std::string &targetFileName) {
//   // 1. If a file path argument was specified, initialize its workspace
//   context if (!targetFileName.empty()) {
//     if (fileManager.ifFileExists(targetFileName)) {
//       fileManager.loadFile(targetFileName, buffer);
//     } else {
//       // Create a blank placeholder file if it doesn't exist yet
//       fileManager.saveFile(targetFileName, buffer);
//       fileManager.loadFile(targetFileName, buffer);
//     }
//     setupFileContext();
//   }

//   // 2. Instantiate terminal configuration (Automatically calls
//   enableRawMode()
//   // in its constructor)
//   Terminal term;

//   // 3. Initial sync pass to establish baseline layout maps
//   int initialVisibleHeight = term.getScreenRows() - 2;
//   updateStatus(initialVisibleHeight);

//   // 4. Main Cross-Platform Interactive Processing Engine Loop
//   while (ifEditorRunning) {
//     // Dynamically query terminal size constraints to correctly configure
//     line
//     // viewport bounds
//     int visibleTextHeight = term.getScreenRows() - 2;
//     updateStatus(visibleTextHeight);

//     // Refresh display layout output stream
//     term.render(*this);

//     // Block process execution and poll for user input events
//     InputKey keyInput = term.readKey();

//     // Map keystrokes directly down into structural text or navigation
//     // adjustments
//     handleInputFromKeyboard(keyInput);
//   }

//   // When loop terminates, Terminal's destructor runs naturally here,
//   // calling disableRawMode() and restoring the clean terminal view state.
// }

void TextEditor::setupFileContext() {
  // Pull the total row counts from the freshly populated GapBuffer
  status.totalRows = buffer.getRowCount();

  // Set the status bar text message notifying the user the file loaded
  if (fileManager.isFileOpen()) {
    status.statusMessage = "Loaded " + fileManager.getCurrentFileName();
  }
}

bool TextEditor::getifEditorRunning() { return ifEditorRunning; }

void TextEditor::scrollWindow(size_t terminalTextHeight) {
  if (status.cursorRow - 1 < status.rowOffset) {
    status.rowOffset = status.cursorRow - 1;
  }
  if (status.cursorRow - 1 >= status.rowOffset + terminalTextHeight) {
    status.rowOffset = status.cursorRow - terminalTextHeight;
  }
}

void TextEditor::handleInputFromKeyboard(InputKey k) {
  // checks if ESC key is the given 'k' parameter
  if (k == InputKey::ESCAPE) {
    status.currMode = EditorMode::NORMAL;
    status.lastCommand = "";
    status.pendingCount = 0;
    return;
  }

  switch (status.currMode) {
  case EditorMode::NORMAL:
    executeNormalCommand(k);
    break;
  case EditorMode::INSERT:
    executeInsertCommand(k);
    break;
  case EditorMode::COMMAND:
    handleCommandModeInput(k);
    break;
  }
}

void TextEditor::executeNormalCommand(InputKey k) {
  if (k == InputKey::ENTER) {
    status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
    buffer.moveCursorDown(status.targetColumn);
    buffer.moveCursorToRowStart();
    return;
  }

  // Persistent state flags for tracking multi-key sequences
  static bool dPressed = false;
  static bool yPressed = false;
  static bool gPressed = false;
  static bool greaterPressed = false;
  static bool lessPressed = false;

  // =============================================================
  // 1. BRIDGE HARDWARE KEYS & ALPHA KEYS TO SINGLECHARCMD
  // =============================================================
  SingleCharCmd act = SingleCharCmd::None;
  char key = '\0';

  if (k == InputKey::ARROW_UP) {
    act = SingleCharCmd::MoveUp;
    key = 'k';
  } else if (k == InputKey::ARROW_DOWN) {
    act = SingleCharCmd::MoveDown;
    key = 'j';
  } else if (k == InputKey::ARROW_LEFT || k == InputKey::BACKSPACE) {
    act = SingleCharCmd::MoveLeft;
    key = 'h';
  } else if (k == InputKey::ARROW_RIGHT) {
    act = SingleCharCmd::MoveRight;
    key = 'l';
  } else if (k == InputKey::DELETE_KEY) {
    act = SingleCharCmd::DeleteCharAfterCursor;
    key = 'x';
  } else {
    key = static_cast<char>(k);
    act = static_cast<SingleCharCmd>(key);
  }

  // =============================================================
  // 2. ACCUMULATE DIGITS FOR REPETITIONS
  // =============================================================
  if (isdigit(key)) {
    if (status.pendingCount == 0 && act == SingleCharCmd::MoveToRowStart) {
      buffer.moveCursorToRowStart();
      status.targetColumn = 0;
      dPressed = yPressed = gPressed = greaterPressed = lessPressed = false;
      return;
    }
    status.pendingCount = (status.pendingCount * 10) + (key - '0');
    return;
  }
  int count = (status.pendingCount == 0) ? 1 : status.pendingCount;

  // =============================================================
  // 3. RESOLVE MULTI-KEY SEQUENCES
  // =============================================================
  DoubleCharCmd action = DoubleCharCmd::None;
  if (dPressed && key == 'd') {
    action = DoubleCharCmd::DeleteRow;
  } else if (yPressed && key == 'y') {
    action = DoubleCharCmd::YankRow;
  } else if (gPressed && key == 'g') {
    action = DoubleCharCmd::GoTowardsFileTop;
  } else if (greaterPressed && key == '>') {
    action = DoubleCharCmd::IndentRight;
  } else if (lessPressed && key == '<') {
    action = DoubleCharCmd::IndentLeft;
  }

  // ------------------------------------------------------------
  // 4. Execute multi-key actions
  // ------------------------------------------------------------
  if (action != DoubleCharCmd::None) {
    switch (action) {
    case DoubleCharCmd::DeleteRow: {
      if (!buffer.isBufferEmpty()) {
        size_t currentLineNum = buffer.getCurrentRowIndex(); // 1-based index
        size_t rowsAvailable = (status.totalRows >= currentLineNum)
                                   ? (status.totalRows - currentLineNum + 1)
                                   : 0;
        int actualDeleteCount = (count > static_cast<int>(rowsAvailable))
                                    ? static_cast<int>(rowsAvailable)
                                    : count;

        size_t startRowIndex = currentLineNum - 1;
        size_t blockStart = buffer.getRowStartIndex(startRowIndex);
        size_t blockEnd = blockStart;

        // CHANGED: Pre-calculate the text block spans across ALL lines to
        // delete
        bool includesPreviousNewline = false;
        for (int i = 0; i < actualDeleteCount; ++i) {
          size_t currRow = startRowIndex + i;
          if (currRow >= buffer.getRowCount())
            break;
          blockEnd = buffer.getRowEndIndex(currRow);
        }

        // Handle text extraction layout variations matching deleteRow edge
        // cases
        string deletedBlockText = "";
        if (startRowIndex + actualDeleteCount >= buffer.getRowCount() &&
            startRowIndex > 0) {
          // Deleting up to the bottom row implies pulling back space characters
          includesPreviousNewline = true;
          deletedBlockText = buffer.getTextInRange(blockStart - 1, blockEnd);
        } else if (blockEnd < buffer.getTextLength() &&
                   buffer.getCharAt(blockEnd) == '\n') {
          deletedBlockText = buffer.getTextInRange(blockStart, blockEnd + 1);
        } else {
          deletedBlockText = buffer.getTextInRange(blockStart, blockEnd);
        }

        // Execute raw internal buffer line structural deletions safely
        for (int i = 0; i < actualDeleteCount; ++i) {
          deleteRow();
        }
        fileManager.markAsModified();

        // CHANGED: Push only ONE compound action block onto stack
        size_t pushPos =
            includesPreviousNewline ? (blockStart - 1) : blockStart;
        undoStack.push(
            UndoAction(EditType::INSERT_CHAR, pushPos, deletedBlockText));
        redoStack.clear();
      }
      break;
    }

    case DoubleCharCmd::YankRow: {
      string yanked = "";
      size_t oldCursor = buffer.getCursorPosition();
      for (int i = 0; i < count; ++i) {
        yankRow();
        yanked += yankBuffer;
        buffer.moveCursorDown(
            buffer.getColIndexFromPos(buffer.getCursorPosition()));
      }
      yankBuffer = yanked;
      buffer.moveCursorToPos(oldCursor);
      break;
    }

    case DoubleCharCmd::GoTowardsFileTop: {
      if (count > 1) {
        buffer.moveCursorToSpecificRow(count - 1);
      } else {
        buffer.moveCursorToBufferStart();
      }
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
      break;
    }

    case DoubleCharCmd::IndentRight: {
      if (!buffer.isBufferEmpty()) {
        size_t initialCursorPos = buffer.getCursorPosition();
        size_t startingRowIndex = buffer.getCurrentRowIndex() - 1; // 0-based
        size_t totalLines = buffer.getRowCount();

        // Cap the count if it goes past the end of the file
        int linesToIndent = count;
        if (startingRowIndex + linesToIndent > totalLines) {
          linesToIndent = totalLines - startingRowIndex;
        }

        // We can capture individual lines but push them sequentially, or group
        // them. To seamlessly match your current undo engine without
        // alterations:
        for (int i = 0; i < linesToIndent; ++i) {
          size_t currentRow = startingRowIndex + i;

          // Move cursor internally to the start of the row we want to indent
          buffer.moveCursorToSpecificRow(currentRow);
          size_t rowFirstIndex = buffer.getRowStartIndex(currentRow);

          indentRow(true);
          fileManager.markAsModified();

          // Push undo frame for this specific line
          undoStack.push(
              UndoAction(EditType::DELETE_CHAR, rowFirstIndex, "    "));
        }

        redoStack.clear();
        // Restore cursor layout state back to where the user was working
        buffer.moveCursorToPos(initialCursorPos);
      }
      break;
    }

    case DoubleCharCmd::IndentLeft: {
      if (!buffer.isBufferEmpty()) {
        size_t initialCursorPos = buffer.getCursorPosition();
        size_t startingRowIndex = buffer.getCurrentRowIndex() - 1; // 0-based
        size_t totalLines = buffer.getRowCount();

        int linesToUnindent = count;
        if (startingRowIndex + linesToUnindent > totalLines) {
          linesToUnindent = totalLines - startingRowIndex;
        }

        for (int i = 0; i < linesToUnindent; ++i) {
          size_t currentRow = startingRowIndex + i;

          buffer.moveCursorToSpecificRow(currentRow);
          size_t rowFirstIndex = buffer.getRowStartIndex(currentRow);

          // Only alter tracking state and mutate if the row safely contains
          // leading spaces
          if (buffer.getTextInRange(rowFirstIndex, rowFirstIndex + 4) ==
              "    ") {
            indentRow(false);
            fileManager.markAsModified();

            undoStack.push(
                UndoAction(EditType::INSERT_CHAR, rowFirstIndex, "    "));
          }
        }

        redoStack.clear();
        buffer.moveCursorToPos(initialCursorPos);
      }
      break;
    }

    default:
      break;
    }

    dPressed = yPressed = gPressed = greaterPressed = lessPressed = false;
    status.pendingCount = 0;
    status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
    updateStatus(22);
    return;
  }

  // =============================================================
  // 5. BEGIN MULTI-KEY SEQUENCE TRACKING
  // =============================================================
  if (key == 'd' || key == 'y' || key == 'g' || key == '>' || key == '<') {
    dPressed = (key == 'd'), yPressed = (key == 'y'), gPressed = (key == 'g'),
    greaterPressed = (key == '>'), lessPressed = (key == '<');
    return;
  } else {
    dPressed = yPressed = gPressed = greaterPressed = lessPressed = false;
  }

  // ------------------------------------------------------------
  // 6. Execute repeatable single-key commands
  // ------------------------------------------------------------
  status.pendingCount = 0;

  for (int i = 0; i < count; ++i) {
    switch (act) {
    case SingleCharCmd::EnterInsertMode: {
      status.currMode = EditorMode::INSERT;
      return;
    }

    case SingleCharCmd::EnterCommandMode: {
      status.currMode = EditorMode::COMMAND;
      status.lastCommand = "";
      return;
    }

    case SingleCharCmd::MoveDown: {
      buffer.moveCursorDown(status.targetColumn);
      break;
    }

    case SingleCharCmd::MoveUp: {
      buffer.moveCursorUp(status.targetColumn);
      break;
    }

    case SingleCharCmd::MoveLeft: {
      buffer.moveCursorLeft();
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
      break;
    }

    case SingleCharCmd::MoveRight: {
      buffer.moveCursorRight();
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
      break;
    }

    case SingleCharCmd::MoveByWordForward: {
      buffer.moveCursorByWord(true);
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
      break;
    }

    case SingleCharCmd::MoveByWordBackward: {
      buffer.moveCursorByWord(false);
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
      break;
    }

    case SingleCharCmd::MoveToWordEnd: {
      buffer.moveCursorToEndOfWord();
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
      break;
    }

    case SingleCharCmd::MoveToRowEnd: {
      buffer.moveCursorToRowEnd();
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
      break;
    }

    case SingleCharCmd::GoTowardsFileBottom: {
      if (count > 1) {
        buffer.moveCursorToSpecificRow(count - 1);
        i = count;
      } else {
        buffer.moveCursorToBufferEnd();
      }
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
      break;
    }

    case SingleCharCmd::DeleteCharAfterCursor: {
      if (buffer.getCursorPosition() < buffer.getTextLength()) {
        size_t changePos = buffer.getCursorPosition();
        char deletedChar = buffer.getCharAt(changePos);
        buffer.deleteCharAfterCursor();
        fileManager.markAsModified();
        undoStack.push(UndoAction(EditType::INSERT_CHAR, changePos,
                                  string(1, deletedChar)));
        redoStack.clear();
        status.targetColumn =
            buffer.getColIndexFromPos(buffer.getCursorPosition());
      }
      break;
    }

    case SingleCharCmd::DeleteToEndOfRow: {
      if (!buffer.isBufferEmpty()) {
        size_t cursorPos = buffer.getCursorPosition();
        size_t rowEnd = buffer.getRowEndIndex(buffer.getCurrentRowIndex() - 1);
        if (rowEnd > cursorPos) {
          string deletedText = buffer.getTextInRange(cursorPos, rowEnd);
          deleteToEndOfRow();
          fileManager.markAsModified();
          undoStack.push(
              UndoAction(EditType::INSERT_CHAR, cursorPos, deletedText));
          redoStack.clear();
          status.targetColumn =
              buffer.getColIndexFromPos(buffer.getCursorPosition());
        }
      }
      break;
    }

    case SingleCharCmd::JoinRows: {
      size_t rowIndex = buffer.getCurrentRowIndex() - 1;
      if (rowIndex < buffer.getRowCount() - 1) {
        buffer.moveCursorToRowEnd();
        size_t joinPos = buffer.getCursorPosition();
        joinCurrentRow();
        fileManager.markAsModified();
        undoStack.push(UndoAction(EditType::BACKSPACE_LN, joinPos, "\n"));
        redoStack.clear();
        status.targetColumn =
            buffer.getColIndexFromPos(buffer.getCursorPosition());
      }
      break;
    }

    case SingleCharCmd::PasteAfter: {
      if (!yankBuffer.empty()) {
        size_t rowEnd = buffer.getRowEndIndex(buffer.getCurrentRowIndex() - 1);
        size_t pastePos = rowEnd + 1;
        bool addedNewline = (rowEnd == buffer.getTextLength());

        pasteStr(true);
        fileManager.markAsModified();

        if (addedNewline) {
          undoStack.push(
              UndoAction(EditType::DELETE_CHAR, rowEnd, "\n" + yankBuffer));
        } else {
          string pasteCheck = yankBuffer;
          if (!pasteCheck.empty() && pasteCheck.back() == '\n')
            pasteCheck.pop_back();
          undoStack.push(
              UndoAction(EditType::DELETE_CHAR, pastePos, pasteCheck));
        }
        redoStack.clear();
        status.targetColumn =
            buffer.getColIndexFromPos(buffer.getCursorPosition());
      }
      break;
    }

    case SingleCharCmd::PasteBefore: {
      if (!yankBuffer.empty()) {
        size_t rowStart =
            buffer.getRowStartIndex(buffer.getCurrentRowIndex() - 1);
        pasteStr(false);
        fileManager.markAsModified();
        undoStack.push(UndoAction(EditType::DELETE_CHAR, rowStart, yankBuffer));
        redoStack.clear();
        status.targetColumn =
            buffer.getColIndexFromPos(buffer.getCursorPosition());
      }
      break;
    }

    case SingleCharCmd::YankToEndOfRow: {
      yankToEndOfRow();
      break;
    }

    case SingleCharCmd::Undo: {
      undo();
      return;
    }

    case SingleCharCmd::Redo: {
      redo();
      return;
    }

    default: {
      break;
    }
    }
  }
}

void TextEditor::executeInsertCommand(InputKey k) {
  // =============================================================
  // 1. HARDWARE KEY OVERRIDES (STRUCTURAL ACTIONS)
  // =============================================================
  if (k == InputKey::ENTER) {
    size_t changePos = buffer.getCursorPosition();
    buffer.insertCharAtCursor('\n');
    fileManager.markAsModified();
    status.targetColumn = 0;

    undoStack.push(UndoAction(EditType::ENTER_LN, changePos, "\n"));
    redoStack.clear(); // Typing a new character breaks old redo history paths
    updateStatus(22);
    return;
  } else if (k == InputKey::BACKSPACE) {
    size_t changePos = buffer.getCursorPosition();
    if (changePos > 0) {
      char deletedChar = buffer.getCharAt(changePos - 1);
      buffer.deleteCharBeforeCursor();
      fileManager.markAsModified();

      if (deletedChar == '\n') {
        undoStack.push(UndoAction(EditType::BACKSPACE_LN, changePos - 1, "\n"));
      } else {
        undoStack.push(UndoAction(EditType::INSERT_CHAR, changePos - 1,
                                  string(1, deletedChar)));
      }
      redoStack.clear();
      // CHANGED: Keep track of target layout position
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
    }
    updateStatus(22);
    return;
  } else if (k == InputKey::DELETE_KEY) {
    size_t changePos = buffer.getCursorPosition();
    if (changePos < buffer.getTextLength()) {
      char deletedChar = buffer.getCharAt(changePos);
      buffer.deleteCharAfterCursor();
      fileManager.markAsModified();

      if (deletedChar == '\n') {
        undoStack.push(UndoAction(EditType::BACKSPACE_LN, changePos, "\n"));
      } else {
        undoStack.push(UndoAction(EditType::INSERT_CHAR, changePos,
                                  string(1, deletedChar)));
      }
      redoStack.clear();
      // CHANGED: Keep track of target layout position
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
    }
    updateStatus(22);
    return;
  } else if (k == InputKey::TAB) {
    size_t changePos = buffer.getCursorPosition();
    buffer.insertStringAtCursor("    ");
    fileManager.markAsModified();

    // CHANGED: Add structural tracking for Tab character spaces
    undoStack.push(UndoAction(EditType::DELETE_CHAR, changePos, "    "));
    redoStack.clear();
    status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
    updateStatus(22);
    return;
  }

  // =============================================================
  // 2. MID-INSERTION CURSOR NAVIGATION MECHANISMS
  // =============================================================
  if (k == InputKey::ARROW_UP) {
    // Reuse existing persistent target
    buffer.moveCursorUp(status.targetColumn);
    updateStatus(22);
    return;
  } else if (k == InputKey::ARROW_DOWN) {
    buffer.moveCursorDown(status.targetColumn);
    updateStatus(22);
    return;
  } else if (k == InputKey::ARROW_LEFT) {
    buffer.moveCursorLeft();
    status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
    updateStatus(22);
    return;
  } else if (k == InputKey::ARROW_RIGHT) {
    buffer.moveCursorRight();
    status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
    updateStatus(22);
    return;
  }

  // =============================================================
  // 3. LITERAL ALPHANUMERIC & TEXTUAL CHARACTER PACKING
  // =============================================================
  // Strip away the InputKey enum packaging down to raw char byte code
  char rawChar = static_cast<char>(k);

  // Filter for printable ASCII codes (Range 32 space through 126 tilde
  // symbol)
  if (rawChar >= 32 && rawChar <= 126) {
    size_t changePos = buffer.getCursorPosition();
    buffer.insertCharAtCursor(rawChar);
    fileManager.markAsModified();

    status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());

    // CHUNKING LOGIC: Try to merge with the existing continuous typing
    // block
    UndoAction *topAction = undoStack.peek();

    // Condition to merge:
    // 1. There is an action on the stack
    // 2. Its type is DELETE_CHAR (meaning it was typed text)
    // 3. The new character is being typed exactly at the end of the
    // previous string
    if (topAction && topAction->type == EditType::DELETE_CHAR &&
        changePos == topAction->position + topAction->text.length()) {
      topAction->text += rawChar; // Append to current chunk!
    } else {
      // Otherwise, it's a new separate action (e.g., user clicked elsewhere
      // or restarted typing)
      undoStack.push(
          UndoAction(EditType::DELETE_CHAR, changePos, string(1, rawChar)));
    }
    redoStack.clear();
    updateStatus(22);
  }
}

void TextEditor::handleCommandModeInput(InputKey k) {
  // =============================================================
  // 1. PROCESS ACTION/SUBMISSION KEYS
  // =============================================================
  if (k == InputKey::ENTER) {
    // Process the accumulated string (skipping the initial ':' marker)
    executeCommandMode(status.lastCommand);
    return;
  }

  if (k == InputKey::BACKSPACE) {
    if (!status.lastCommand.empty()) {
      status.lastCommand.pop_back();
    } else {
      // Vim Behavior: If you backspace on an empty command line, drop out
      // to normal mode
      status.currMode = EditorMode::NORMAL;
    }
    return;
  }

  // =============================================================
  // 2. APPEND LITERAL PRINTABLE ALPHANUMERIC CHARACTERS
  // =============================================================
  char rawChar = static_cast<char>(k);

  // Filter for printable text and symbols (space through tilde)
  if (rawChar >= 32 && rawChar <= 126) {
    status.lastCommand += rawChar;
  }
}

void TextEditor::executeCommandMode(const string &rawCmd) {
  // Clean command prefix if it contains the leading ':' marker
  string cmd = rawCmd;
  if (!cmd.empty() && cmd[0] == ':') {
    cmd = cmd.substr(1);
  }

  // If the user hits enter on a completely empty prompt, return quietly to
  // normal mode
  if (cmd.empty()) {
    status.currMode = EditorMode::NORMAL;
    return;
  }

  // =============================================================
  // 1. SPLIT COMMAND INTO COMPONENT PARTS
  // =============================================================
  size_t spacePos = cmd.find(' ');
  string action, arg;

  if (spacePos < cmd.length()) {
    action = cmd.substr(0, spacePos);
    arg = cmd.substr(spacePos + 1);
  } else {
    action = cmd;
    arg = "";
  }

  // =============================================================
  // 2. PARSE AND RESOLVE OPERATIONS
  // =============================================================

  // --- :q - Quit safely with unsaved change guard ---
  if (action == "q") {
    if (fileManager.hasUnsavedChanges()) {
      status.statusMessage = "Error: Unsaved changes! Use :q! to force quit";
      status.currMode = EditorMode::NORMAL;
    } else {
      ifEditorRunning = false;
    }
    return;
  }

  // --- :q! - Force quit ignoring unwritten changes ---
  if (action == "q!") {
    ifEditorRunning = false;
    return;
  }

  // --- :w - Save current active modifications to file ---
  if (action == "w") {
    string filename = arg.empty() ? fileManager.getCurrentFileName() : arg;
    if (filename.empty()) {
      status.statusMessage = "Error: No filename provided";
    } else if (fileManager.saveFile(filename, buffer)) {
      status.statusMessage = "Saved as " + filename;
    } else {
      status.statusMessage = "Error: Could not save file!";
    }
    status.currMode = EditorMode::NORMAL;
    return;
  }

  // --- :wq - Save changes and immediately close out editor ---
  if (action == "wq") {
    string filename = arg.empty() ? fileManager.getCurrentFileName() : arg;
    if (filename.empty()) {
      status.statusMessage = "Error: No filename provided";
      status.currMode = EditorMode::NORMAL;
    } else if (fileManager.saveFile(filename, buffer)) {
      status.statusMessage = "Saved as " + filename + " and quitting";
      ifEditorRunning = false;
    } else {
      status.statusMessage = "Error: Could not save file!";
      status.currMode = EditorMode::NORMAL;
    }
    return;
  }

  // --- :e - Clear buffer and read alternative file onto screen ---
  if (action == "e") {
    if (arg.empty()) {
      status.statusMessage = "Error: No filename provided for :e";
    } else if (fileManager.hasUnsavedChanges()) {
      status.statusMessage =
          "Error: Unsaved changes! Save first or use :e! filename";
    } else if (fileManager.loadFile(arg, buffer)) {
      status.statusMessage = "Loaded " + arg;
      undoStack.clear(); // <-- Reset
      redoStack.clear(); // <-- Reset
      // setupFileContext();
      updateStatus(22);
    } else {
      status.statusMessage =
          "Error: File not found. Created blank file: " + arg;
      buffer.clearBuffer();
      fileManager.saveFile(arg, buffer);
      fileManager.loadFile(arg, buffer);
      undoStack.clear(); // <-- Reset
      redoStack.clear(); // <-- Reset
      updateStatus(22);
    }
    status.currMode = EditorMode::NORMAL;
    return;
  }

  // --- :e! - FORCE RELOAD OR FORCE OPEN (Discards unsaved changes) ---
  if (action == "e!") {
    if (arg.empty()) {
      // Case 1: ':e!' with no filename -> Reload the active file from disk
      string currentFile = fileManager.getCurrentFileName();
      if (currentFile.empty()) {
        status.statusMessage = "Error: No active filename to reload";
      } else if (fileManager.loadFile(currentFile, buffer)) {
        status.statusMessage =
            "Reloaded " + currentFile + " from disk (changes dropped)";
        undoStack.clear(); // <-- Reset
        redoStack.clear(); // <-- Reset
        // setupFileContext();
        updateStatus(22);
      } else {
        status.statusMessage = "Error: Could not reload file!";
      }
    } else {
      // Case 2: ':e! filename' -> Open a file, ignore unsaved changes
      if (fileManager.loadFile(arg, buffer)) {
        status.statusMessage = "Loaded " + arg + " (forced)";
        undoStack.clear(); // <-- Reset
        redoStack.clear(); // <-- Reset
        setupFileContext();
        updateStatus(22);
      } else {
        // Vim behavior fallback: if file doesn't exist, switch to it as an
        // empty workspace
        status.statusMessage =
            "Error: File not found. Switched to new context: " + arg;
        buffer.clearBuffer();
        fileManager.saveFile(arg, buffer); // Create the empty file
        fileManager.loadFile(arg, buffer);
        undoStack.clear(); // <-- Reset
        redoStack.clear(); // <-- Reset
        setupFileContext();
        updateStatus(22);
      }
    }
    status.currMode = EditorMode::NORMAL;
    return;
  }

  // --- :d N or :d - Delete N lines ---
  if (action == "d") {
    // Default to deleting the current line if no number is given
    int linesToDelete = 1;

    if (!arg.empty()) {
      // Check if the argument is a valid number
      bool isNumber = true;
      for (char c : arg) {
        if (!isdigit(c)) {
          isNumber = false;
          break;
        }
      }

      if (isNumber) {
        linesToDelete = stoi(arg);
      } else {
        status.statusMessage = "Error: Invalid line count for delete";
        status.currMode = EditorMode::NORMAL;
        return;
      }
    }

    if (!buffer.isBufferEmpty()) {
      size_t currentLineNum = buffer.getCurrentRowIndex();
      size_t rowsAvailable = (buffer.getRowCount() >= currentLineNum)
                                 ? (buffer.getRowCount() - currentLineNum + 1)
                                 : 0;
      int actualDeleteCount = (linesToDelete > static_cast<int>(rowsAvailable))
                                  ? static_cast<int>(rowsAvailable)
                                  : linesToDelete;

      size_t startRowIndex = currentLineNum - 1;
      size_t blockStart = buffer.getRowStartIndex(startRowIndex);
      size_t blockEnd = blockStart;

      bool includesPreviousNewline = false;
      for (int i = 0; i < actualDeleteCount; ++i) {
        size_t currRow = startRowIndex + i;
        if (currRow >= buffer.getRowCount())
          break;
        blockEnd = buffer.getRowEndIndex(currRow);
      }

      string deletedBlockText = "";
      if (startRowIndex + actualDeleteCount >= buffer.getRowCount() &&
          startRowIndex > 0) {
        includesPreviousNewline = true;
        deletedBlockText = buffer.getTextInRange(blockStart - 1, blockEnd);
      } else if (blockEnd < buffer.getTextLength() &&
                 buffer.getCharAt(blockEnd) == '\n') {
        deletedBlockText = buffer.getTextInRange(blockStart, blockEnd + 1);
      } else {
        deletedBlockText = buffer.getTextInRange(blockStart, blockEnd);
      }

      for (int i = 0; i < actualDeleteCount; ++i) {
        deleteRow();
      }
      fileManager.markAsModified();

      size_t pushPos = includesPreviousNewline ? (blockStart - 1) : blockStart;
      undoStack.push(
          UndoAction(EditType::INSERT_CHAR, pushPos, deletedBlockText));
      redoStack
          .clear(); // Ensure history tree is broken cleanly on modifications
      status.statusMessage =
          "Deleted " + to_string(actualDeleteCount) + " line(s)";
    }

    status.currMode = EditorMode::NORMAL;
    status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
    updateStatus(22);
    return;
  }

  // =============================================================
  // 3. UNKNOWN UTILITY FALL-THROUGH
  // =============================================================
  status.statusMessage = "Error: Unknown command: " + action;
  status.currMode = EditorMode::NORMAL;
}

void TextEditor::undo() {
  if (undoStack.isEmpty()) {
    status.statusMessage = "Already at oldest change";
    return;
  }

  UndoAction action = undoStack.pop();
  size_t textLen = action.text.length();

  switch (action.type) {
  case EditType::DELETE_CHAR: {
    buffer.moveGapTo(action.position);
    buffer.deleteTextInRange(action.position, action.position + textLen);
    redoStack.push(action);
    break;
  }
  case EditType::ENTER_LN: {
    buffer.moveGapTo(action.position);
    buffer.deleteTextInRange(action.position, action.position + 1);
    redoStack.push(action);
    break;
  }
  case EditType::INSERT_CHAR: {
    buffer.moveGapTo(action.position);
    buffer.insertStringAtPos(action.position, action.text);
    redoStack.push(action);
    break;
  }
  case EditType::BACKSPACE_LN: {
    buffer.moveGapTo(action.position);
    buffer.insertStringAtPos(action.position, action.text);
    redoStack.push(action);
    break;
  }
  }

  // CHANGED: Force gap sync and layout anchors to match post-action
  // positions exactly
  buffer.moveGapTo(action.position);
  status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
  updateStatus(22);
  status.statusMessage = "Undo executed";
}
void TextEditor::redo() {
  if (redoStack.isEmpty()) {
    status.statusMessage = "Already at newest change";
    return;
  }

  UndoAction action = redoStack.pop();
  size_t textLen = action.text.length();

  switch (action.type) {
  case EditType::DELETE_CHAR: {
    buffer.moveGapTo(action.position);
    buffer.insertStringAtPos(action.position, action.text);
    undoStack.push(action);
    buffer.moveGapTo(action.position + textLen);
    break;
  }
  case EditType::ENTER_LN: {
    buffer.moveGapTo(action.position);
    buffer.insertStringAtPos(action.position, "\n");
    undoStack.push(action);
    buffer.moveGapTo(action.position + 1);
    break;
  }
  case EditType::INSERT_CHAR: {
    buffer.moveGapTo(action.position);
    buffer.deleteTextInRange(action.position, action.position + textLen);
    undoStack.push(action);
    buffer.moveGapTo(action.position);
    break;
  }
  case EditType::BACKSPACE_LN: {
    buffer.moveGapTo(action.position);
    buffer.deleteTextInRange(action.position, action.position + 1);
    undoStack.push(action);
    buffer.moveGapTo(action.position);
    break;
  }
  }

  // CHANGED: Standardize target positions cleanly
  buffer.moveGapTo(action.position);
  status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
  updateStatus(22);
  status.statusMessage = "Redo executed";
}
void TextEditor::joinCurrentRow() {
  // 1. Move cursor to the end of the current line
  buffer.moveCursorToRowEnd();
  // Replace '\n' at lineEnd with a space.
  buffer.deleteTextInRange(buffer.getCursorPosition(),
                           buffer.getCursorPosition() + 1);
  // buffer.moveGapTo(endIdx);  CHECK THIS
  buffer.insertCharAtCursor(' ');
}

void TextEditor::deleteRow() {
  if (buffer.isBufferEmpty()) {
    return;
  }

  size_t rowIndex = buffer.getCurrentRowIndex() - 1;
  size_t rowStart = buffer.getRowStartIndex(rowIndex);
  size_t rowEnd = buffer.getRowEndIndex(rowIndex);
  size_t totalRows = buffer.getRowCount();

  // CHANGED: Edge Case 1 - If it's the only row in the entire file
  if (totalRows == 1) {
    buffer.clearBuffer();
    buffer.moveGapTo(0);
    status.targetColumn = 0;
    return;
  }

  // CHANGED: Edge Case 2 - If it is the last row in the file (but not the
  // only row)
  if (rowIndex == totalRows - 1) {
    // Delete the previous line's newline character along with this line
    buffer.deleteTextInRange(rowStart - 1, rowEnd);

    // Move cursor to the beginning of the new last line (the line above)
    size_t prevRowIndex = rowIndex - 1;
    size_t prevRowStart = buffer.getRowStartIndex(prevRowIndex);
    buffer.moveGapTo(prevRowStart);
    status.targetColumn = 0;
    return;
  }

  // Standard case: Middle lines (Delete line text + its trailing newline)
  if (rowEnd < buffer.getTextLength() && buffer.getCharAt(rowEnd) == '\n') {
    buffer.deleteTextInRange(rowStart, rowEnd + 1);
  } else {
    buffer.deleteTextInRange(rowStart, rowEnd);
  }

  // Snap cursor to the start of the line that moves into this slot
  buffer.moveGapTo(rowStart);
  status.targetColumn = 0;
}

void TextEditor::deleteToEndOfRow() {
  if (buffer.isBufferEmpty()) {
    return;
  }

  size_t cursorPos = buffer.getCursorPosition();
  size_t rowEnd = buffer.getRowEndIndex(buffer.getCurrentRowIndex() - 1);

  // D deletes up to the end of the line
  if (rowEnd > cursorPos) {
    buffer.deleteTextInRange(cursorPos, rowEnd);
  }
}

void TextEditor::yankRow() {
  size_t rowIndex = buffer.getCurrentRowIndex() - 1;
  yankBuffer = buffer.getRowText(rowIndex);
  if (yankBuffer.empty() || yankBuffer.back() != '\n') {
    yankBuffer += '\n';
  }
  status.statusMessage = "Yanked line " + to_string(rowIndex + 1);
}

void TextEditor::yankToEndOfRow() {
  // get cursor position
  size_t cursorPos = buffer.getCursorPosition();
  // get the end of the current row
  size_t rowEnd = buffer.getRowEndIndex(buffer.getCurrentRowIndex() - 1);

  // extract the range and store in clipboard
  yankBuffer = buffer.getTextInRange(cursorPos, rowEnd);
  if (yankBuffer.empty() || yankBuffer.back() != '\n') {
    yankBuffer += '\n';
  }

  status.statusMessage = "Yanked to end of row";
}

void TextEditor::pasteStr(bool after) {
  if (yankBuffer.empty()) {
    status.statusMessage = "Nothing to paste!";
    return;
  }

  if (after) {
    // p
    // 1. Get the newline character of the current line
    size_t rowEnd = buffer.getRowEndIndex(buffer.getCurrentRowIndex() - 1);

    // 2. Insert AFTER that newline.
    // If rowEnd == getTextLength(), it means we are at the last line,
    // so we need to manually insert a '\n' before pasting.
    if (rowEnd == buffer.getTextLength()) {
      buffer.insertStringAtPos(rowEnd, "\n");
    }

    string yanked = yankBuffer;
    if (!yanked.empty() && yanked.back() == '\n') {
      yanked.pop_back();
    }

    buffer.insertStringAtPos(rowEnd + 1, yanked);
    buffer.moveGapTo(rowEnd + 1);
  } else {
    // P

    // 1. Paste BEFORE current line
    size_t rowStart = buffer.getRowStartIndex(buffer.getCurrentRowIndex() - 1);
    buffer.insertStringAtPos(rowStart, yankBuffer);
    buffer.moveGapTo(rowStart);
  }
}

void TextEditor::indentRow(bool right) {
  size_t rowFirstIndex =
      buffer.getRowStartIndex(buffer.getCurrentRowIndex() - 1);

  if (right) {
    buffer.insertStringAtPos(rowFirstIndex, "    "); // Insert 4 spaces
    buffer.moveCursorToRowEnd();
  } else {
    // Only unindent if the first few chars are actually spaces
    if (buffer.getTextInRange(rowFirstIndex, rowFirstIndex + 4) == "    ") {
      buffer.deleteTextInRange(rowFirstIndex, rowFirstIndex + 4);
      buffer.moveCursorToRowEnd();
    }
  }
}

void TextEditor::updateStatus(size_t terminalTextHeight) {
  status.cursorRow = buffer.getCurrentRowIndex();
  status.cursorCol = buffer.getCurrentColIndex();
  status.totalRows = buffer.getRowCount();
  updateModifiedStatus();
  scrollWindow(terminalTextHeight);
}

void TextEditor::updateModifiedStatus() {
  status.unsavedChanges = fileManager.hasUnsavedChanges();
}

bool TextEditor::attemptQuit() {
  if (fileManager.hasUnsavedChanges()) {
    status.statusMessage = "E: Unsaved changes! Use :q! to force quit";
    return false;
  }
  return true;
}
