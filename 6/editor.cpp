#include "editor.h"

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

void TextEditor::setupFileContext() {
  status.totalRows = buffer.getRowCount();
  // Force reset cursor and offset to start of file
  status.cursorRow = 1;
  status.cursorCol = 1;
  status.rowOffset = 0;
  status.targetColumn = 0;
  buffer.moveCursorToPos(0);

  if (fileManager.isFileOpen()) {
    status.statusMessage = "Loaded " + fileManager.getCurrentFileName();
  }
}

bool TextEditor::getifEditorRunning() { return ifEditorRunning; }

// void TextEditor::scrollWindow(size_t terminalTextHeight) {
//   size_t totalRows = buffer.getRowCount();

//   // Scroll up when cursor goes above visible area
//   if (status.cursorRow - 1 < status.rowOffset) {
//     status.rowOffset = status.cursorRow - 1;
//   }

//   // CHANGED: Scroll down but prevent unsigned wraparound when cursorRow <
//   // terminalTextHeight
//   if (status.cursorRow - 1 >= status.rowOffset + terminalTextHeight) {
//     if (status.cursorRow > terminalTextHeight) {
//       status.rowOffset = status.cursorRow - terminalTextHeight;
//     } else {
//       status.rowOffset = 0; // Keep at top if file is smaller than screen
//     }
//   }

//   // CHANGED: Clamp rowOffset to prevent scrolling past EOF:
//   if (totalRows > terminalTextHeight &&
//       status.rowOffset + terminalTextHeight > totalRows) {
//     status.rowOffset = totalRows - terminalTextHeight;
//   }
// }

void TextEditor::scrollWindow(size_t terminalTextHeight) {
  size_t totalRows = buffer.getRowCount();

  // Scroll up instantly when cursor row goes above row offset
  if (status.cursorRow - 1 < status.rowOffset) {
    status.rowOffset = status.cursorRow - 1;
  }

  // Scroll down smoothly when the cursor tries to push out of view frame
  if (status.cursorRow - 1 >= status.rowOffset + terminalTextHeight) {
    status.rowOffset = status.cursorRow - terminalTextHeight;
  }

  // Clamp window parameters seamlessly to the total line length limits
  if (status.rowOffset + terminalTextHeight > totalRows) {
    if (totalRows >= terminalTextHeight) {
      status.rowOffset = totalRows - terminalTextHeight;
    } else {
      status.rowOffset = 0;
    }
  }
}

void TextEditor::handleInputFromKeyboard(InputKey k) {
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

  static bool dPressed = false;
  static bool yPressed = false;
  static bool gPressed = false;
  static bool greaterPressed = false;
  static bool lessPressed = false;

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

  // --- digit accumulation ---
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

  // --- resolve multi-key sequences ---
  DoubleCharCmd action = DoubleCharCmd::None;
  if (dPressed && key == 'd')
    action = DoubleCharCmd::DeleteRow;
  else if (yPressed && key == 'y')
    action = DoubleCharCmd::YankRow;
  else if (gPressed && key == 'g')
    action = DoubleCharCmd::GoTowardsFileTop;
  else if (greaterPressed && key == '>')
    action = DoubleCharCmd::IndentRight;
  else if (lessPressed && key == '<')
    action = DoubleCharCmd::IndentLeft;

  // --- execute multi-key actions ---
  if (action != DoubleCharCmd::None) {
    switch (action) {

    case DoubleCharCmd::DeleteRow: {
      if (!buffer.isBufferEmpty()) {
        // one group for the whole dd/Ndd command
        UndoGroup::beginGroup();

        size_t currentLineNum = buffer.getCurrentRowIndex();
        size_t rowsAvailable = (status.totalRows >= currentLineNum)
                                   ? (status.totalRows - currentLineNum + 1)
                                   : 0;
        int actualDeleteCount = (count > static_cast<int>(rowsAvailable))
                                    ? static_cast<int>(rowsAvailable)
                                    : count;

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

        for (int i = 0; i < actualDeleteCount; ++i)
          deleteRow();
        fileManager.markAsModified();

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
      if (count > 1)
        buffer.moveCursorToSpecificRow(count - 1);
      else
        buffer.moveCursorToBufferStart();
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
      break;
    }

    case DoubleCharCmd::IndentRight: {
      if (!buffer.isBufferEmpty()) {
        UndoGroup::beginGroup();

        size_t initialCursorPos = buffer.getCursorPosition();
        size_t startingRowIndex = buffer.getCurrentRowIndex() - 1;
        size_t totalLines = buffer.getRowCount();

        int linesToIndent = count;
        if (startingRowIndex + linesToIndent > totalLines)
          linesToIndent = totalLines - startingRowIndex;

        for (int i = 0; i < linesToIndent; ++i) {
          size_t currentRow = startingRowIndex + i;
          buffer.moveCursorToSpecificRow(currentRow);
          size_t rowFirstIndex = buffer.getRowStartIndex(currentRow);

          indentRow(true);
          fileManager.markAsModified();

          // CHANGED: all pushed inside the same group — popGroup() will grab
          // them all
          undoStack.push(
              UndoAction(EditType::DELETE_CHAR, rowFirstIndex, "    "));
        }

        redoStack.clear();
        buffer.moveCursorToPos(initialCursorPos);
      }
      break;
    }

    case DoubleCharCmd::IndentLeft: {
      if (!buffer.isBufferEmpty()) {
        UndoGroup::beginGroup();

        size_t initialCursorPos = buffer.getCursorPosition();
        size_t startingRowIndex = buffer.getCurrentRowIndex() - 1;
        size_t totalLines = buffer.getRowCount();

        int linesToUnindent = count;
        if (startingRowIndex + linesToUnindent > totalLines)
          linesToUnindent = totalLines - startingRowIndex;

        for (int i = 0; i < linesToUnindent; ++i) {
          size_t currentRow = startingRowIndex + i;
          buffer.moveCursorToSpecificRow(currentRow);
          size_t rowFirstIndex = buffer.getRowStartIndex(currentRow);

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

  // --- begin multi-key sequence tracking ---
  if (key == 'd' || key == 'y' || key == 'g' || key == '>' || key == '<') {
    dPressed = (key == 'd');
    yPressed = (key == 'y');
    gPressed = (key == 'g');
    greaterPressed = (key == '>');
    lessPressed = (key == '<');
    return;
  } else {
    dPressed = yPressed = gPressed = greaterPressed = lessPressed = false;
  }

  status.pendingCount = 0;

  // CHANGED: every repeatable single-key command that mutates text gets its OWN
  // group so Nx (delete 5 chars) is still one u, but each individual x is its
  // own u. We beginGroup() once before the loop — the loop may push multiple
  // actions but they all land in the same group.
  bool needsGroup =
      (act == SingleCharCmd::DeleteCharAfterCursor ||
       act == SingleCharCmd::DeleteToEndOfRow ||
       act == SingleCharCmd::JoinRows || act == SingleCharCmd::PasteAfter ||
       act == SingleCharCmd::PasteBefore ||
       act == SingleCharCmd::YankToEndOfRow);
  if (needsGroup)
    UndoGroup::beginGroup();

  for (int i = 0; i < count; ++i) {
    switch (act) {

    case SingleCharCmd::EnterInsertMode: {
      // CHANGED: new group so the insert session that follows gets its own undo
      // slot
      UndoGroup::beginGroup();
      status.currMode = EditorMode::INSERT;
      return;
    }

    case SingleCharCmd::EnterCommandMode: {
      status.currMode = EditorMode::COMMAND;
      status.lastCommand = "";
      return;
    }

    // CHANGED: Handle / (search forward only)
    case SingleCharCmd::SearchFile: {
      status.currMode = EditorMode::COMMAND;
      status.lastCommand = "/";
      return;
    }

    case SingleCharCmd::MoveDown:
      buffer.moveCursorDown(status.targetColumn);
      break;
    case SingleCharCmd::MoveUp:
      buffer.moveCursorUp(status.targetColumn);
      break;

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
      } else
        buffer.moveCursorToBufferEnd();
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

    case SingleCharCmd::NextMatch: // Find Next
      searchEngine.findNext(buffer, status.cursorRow, status.statusMessage);
      break;
    case SingleCharCmd::PrevMatch: // Find Previous
      searchEngine.findPrev(buffer, status.cursorRow, status.statusMessage);
      break;

    default:
      break;
    }
  }
}

void TextEditor::executeInsertCommand(InputKey k) {
  if (k == InputKey::ENTER) {
    size_t changePos = buffer.getCursorPosition();
    buffer.insertCharAtCursor('\n');
    fileManager.markAsModified();
    status.targetColumn = 0;

    // CHANGED: Enter gets its own isolated group — seals off typed text before
    // it, and beginGroup() after ensures the next typed chars start a fresh
    // group too.
    UndoGroup::beginGroup();
    undoStack.push(UndoAction(EditType::ENTER_LN, changePos, "\n"));
    UndoGroup::beginGroup(); // next keystrokes start fresh
    redoStack.clear();
    updateStatus(22);
    return;

  } else if (k == InputKey::BACKSPACE) {
    size_t changePos = buffer.getCursorPosition();
    if (changePos > 0) {
      char deletedChar = buffer.getCharAt(changePos - 1);
      buffer.deleteCharBeforeCursor();
      fileManager.markAsModified();

      // CHANGED: every backspace is its own group — never merges with typed
      // text
      UndoGroup::beginGroup();
      if (deletedChar == '\n') {
        undoStack.push(UndoAction(EditType::BACKSPACE_LN, changePos - 1, "\n"));
      } else {
        undoStack.push(UndoAction(EditType::INSERT_CHAR, changePos - 1,
                                  string(1, deletedChar)));
      }
      UndoGroup::beginGroup(); // next keystrokes start fresh
      redoStack.clear();
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

      // CHANGED: delete key also gets its own group
      UndoGroup::beginGroup();
      if (deletedChar == '\n') {
        undoStack.push(UndoAction(EditType::BACKSPACE_LN, changePos, "\n"));
      } else {
        undoStack.push(UndoAction(EditType::INSERT_CHAR, changePos,
                                  string(1, deletedChar)));
      }
      UndoGroup::beginGroup(); // next keystrokes start fresh
      redoStack.clear();
      status.targetColumn =
          buffer.getColIndexFromPos(buffer.getCursorPosition());
    }
    updateStatus(22);
    return;

  } else if (k == InputKey::TAB) {
    size_t changePos = buffer.getCursorPosition();
    buffer.insertStringAtCursor("    ");
    fileManager.markAsModified();

    undoStack.push(UndoAction(EditType::DELETE_CHAR, changePos, "    "));
    redoStack.clear();
    status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
    updateStatus(22);
    return;
  }

  if (k == InputKey::ARROW_UP) {
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

  char rawChar = static_cast<char>(k);
  if (rawChar >= 32 && rawChar <= 126) {
    size_t changePos = buffer.getCursorPosition();
    buffer.insertCharAtCursor(rawChar);
    fileManager.markAsModified();

    status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());

    // Chunking: merge consecutive typed chars into one undo action
    UndoAction *topAction = undoStack.peek();

    if (topAction && topAction->type == EditType::DELETE_CHAR &&
        topAction->groupId ==
            UndoGroup::get() && // CHANGED: only merge within same group
        changePos == topAction->position + topAction->text.length()) {
      topAction->text += rawChar;
    } else {
      undoStack.push(
          UndoAction(EditType::DELETE_CHAR, changePos, string(1, rawChar)));
    }
    redoStack.clear();
    updateStatus(22);
  }
}

void TextEditor::handleCommandModeInput(InputKey k) {
  if (k == InputKey::ENTER) {
    executeCommandMode(status.lastCommand);
    return;
  }
  if (k == InputKey::BACKSPACE) {
    if (!status.lastCommand.empty()) {
      status.lastCommand.pop_back();
    } else {
      status.currMode = EditorMode::NORMAL;
    }
    return;
  }

  char rawChar = static_cast<char>(k);
  if (rawChar >= 32 && rawChar <= 126) {
    status.lastCommand += rawChar;
  }
}

void TextEditor::executeCommandMode(const string &rawCmd) {
  string cmd = rawCmd;
  if (!cmd.empty() && cmd[0] == '/') {
    string pattern = cmd.substr(1); // Extract pattern after '/'
    performSearch(pattern, true);   // Forward search
    status.currMode = EditorMode::NORMAL;
    return;
  }

  if (!cmd.empty() && cmd[0] == ':') {
    cmd = cmd.substr(1);
  }

  if (cmd.empty()) {
    status.currMode = EditorMode::NORMAL;
    return;
  }

  size_t spacePos = cmd.find(' ');
  string action, arg;
  if (spacePos < cmd.length()) {
    action = cmd.substr(0, spacePos);
    arg = cmd.substr(spacePos + 1);
  } else {
    action = cmd;
    arg = "";
  }

  if (action == "q") {
    if (fileManager.hasUnsavedChanges()) {
      status.statusMessage = "Error: Unsaved changes! Use :q! to force quit";
      status.currMode = EditorMode::NORMAL;
    } else {
      ifEditorRunning = false;
    }
    return;
  }
  if (action == "q!") {
    ifEditorRunning = false;
    return;
  }

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

  if (action == "e") {
    if (arg.empty()) {
      status.statusMessage = "Error: No filename provided for :e";
    } else if (fileManager.hasUnsavedChanges()) {
      status.statusMessage =
          "Error: Unsaved changes! Save first or use :e! filename";
    } else if (fileManager.loadFile(arg, buffer)) {
      status.statusMessage = "Loaded " + arg;
      undoStack.clear();
      redoStack.clear();
      setupFileContext();
      updateStatus(22);
    } else {
      status.statusMessage =
          "Error: File not found. Created blank file: " + arg;
      buffer.clearBuffer();
      fileManager.saveFile(arg, buffer);
      fileManager.loadFile(arg, buffer);
      undoStack.clear();
      redoStack.clear();
      updateStatus(22);
    }
    status.currMode = EditorMode::NORMAL;
    return;
  }

  if (action == "e!") {
    if (arg.empty()) {
      string currentFile = fileManager.getCurrentFileName();
      if (currentFile.empty()) {
        status.statusMessage = "Error: No active filename to reload";
      } else if (fileManager.loadFile(currentFile, buffer)) {
        status.statusMessage =
            "Reloaded " + currentFile + " from disk (changes dropped)";
        undoStack.clear();
        redoStack.clear();
        updateStatus(22);
      } else {
        status.statusMessage = "Error: Could not reload file!";
      }
    } else {
      if (fileManager.loadFile(arg, buffer)) {
        status.statusMessage = "Loaded " + arg + " (forced)";
        undoStack.clear();
        redoStack.clear();
        setupFileContext();
        updateStatus(22);
      } else {
        status.statusMessage =
            "Error: File not found. Switched to new context: " + arg;
        buffer.clearBuffer();
        fileManager.saveFile(arg, buffer);
        fileManager.loadFile(arg, buffer);
        undoStack.clear();
        redoStack.clear();
        setupFileContext();
        updateStatus(22);
      }
    }
    status.currMode = EditorMode::NORMAL;
    return;
  }

  if (action == "d") {
    int linesToDelete = 1;
    if (!arg.empty()) {
      bool isNumber = true;
      for (char c : arg) {
        if (!isdigit(c)) {
          isNumber = false;
          break;
        }
      }
      if (isNumber)
        linesToDelete = stoi(arg);
      else {
        status.statusMessage = "Error: Invalid line count for delete";
        status.currMode = EditorMode::NORMAL;
        return;
      }
    }

    if (!buffer.isBufferEmpty()) {
      // CHANGED: one group for :d N
      UndoGroup::beginGroup();

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

      for (int i = 0; i < actualDeleteCount; ++i)
        deleteRow();
      fileManager.markAsModified();

      size_t pushPos = includesPreviousNewline ? (blockStart - 1) : blockStart;
      undoStack.push(
          UndoAction(EditType::INSERT_CHAR, pushPos, deletedBlockText));
      redoStack.clear();
      status.statusMessage =
          "Deleted " + to_string(actualDeleteCount) + " line(s)";
    }

    status.currMode = EditorMode::NORMAL;
    status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
    updateStatus(22);
    return;
  }

  status.statusMessage = "Error: Unknown command: " + action;
  status.currMode = EditorMode::NORMAL;
}

// +--------------------------------------------------------------------------------------------------+
// UNDO / REDO  — now group-aware
// +--------------------------------------------------------------------------------------------------+

void TextEditor::undo() {
  if (undoStack.isEmpty()) {
    status.statusMessage = "Already at oldest change";
    return;
  }

  // CHANGED: pop the entire group (all actions that share the top groupId)
  std::vector<UndoAction> group = undoStack.popGroup();
  // group[0] is the most recently pushed action of the group (stack order)
  // We replay them in the order they were popped, which reverses the original
  // sequence

  for (UndoAction &action : group) {
    size_t textLen = action.text.length();
    switch (action.type) {
    case EditType::DELETE_CHAR:
      buffer.moveGapTo(action.position);
      buffer.deleteTextInRange(action.position, action.position + textLen);
      break;
    case EditType::ENTER_LN:
      buffer.moveGapTo(action.position);
      buffer.deleteTextInRange(action.position, action.position + 1);
      break;
    case EditType::INSERT_CHAR:
      buffer.moveGapTo(action.position);
      buffer.insertStringAtPos(action.position, action.text);
      break;
    case EditType::BACKSPACE_LN:
      buffer.moveGapTo(action.position);
      buffer.insertStringAtPos(action.position, action.text);
      break;
    }
  }

  // CHANGED: push entire group onto redoStack under a new shared group id
  UndoGroup::beginGroup();
  for (UndoAction &action : group) {
    redoStack.push(action);
  }

  // Restore cursor to position of the earliest action in the group
  size_t restorePos = group.back().position; // back() = first pushed = earliest
  buffer.moveGapTo(restorePos);
  status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
  updateStatus(22);
  status.statusMessage = "Undo";
}

void TextEditor::redo() {
  if (redoStack.isEmpty()) {
    status.statusMessage = "Already at newest change";
    return;
  }

  // CHANGED: pop the entire redo group
  std::vector<UndoAction> group = redoStack.popGroup();

  for (UndoAction &action : group) {
    size_t textLen = action.text.length();
    switch (action.type) {
    case EditType::DELETE_CHAR:
      buffer.moveGapTo(action.position);
      buffer.insertStringAtPos(action.position, action.text);
      buffer.moveGapTo(action.position + textLen);
      break;
    case EditType::ENTER_LN:
      buffer.moveGapTo(action.position);
      buffer.insertStringAtPos(action.position, "\n");
      buffer.moveGapTo(action.position + 1);
      break;
    case EditType::INSERT_CHAR:
      buffer.moveGapTo(action.position);
      buffer.deleteTextInRange(action.position, action.position + textLen);
      buffer.moveGapTo(action.position);
      break;
    case EditType::BACKSPACE_LN:
      buffer.moveGapTo(action.position);
      buffer.deleteTextInRange(action.position, action.position + 1);
      buffer.moveGapTo(action.position);
      break;
    }
  }

  // CHANGED: push group back onto undoStack under a new shared group id
  UndoGroup::beginGroup();
  for (UndoAction &action : group) {
    undoStack.push(action);
  }

  size_t restorePos = group.back().position;
  buffer.moveGapTo(restorePos);
  status.targetColumn = buffer.getColIndexFromPos(buffer.getCursorPosition());
  updateStatus(22);
  status.statusMessage = "Redo";
}

// +--------------------------------------------------------------------------------------------------+
// Text manipulation helpers (unchanged logic)
// +--------------------------------------------------------------------------------------------------+

void TextEditor::joinCurrentRow() {
  buffer.moveCursorToRowEnd();
  buffer.deleteTextInRange(buffer.getCursorPosition(),
                           buffer.getCursorPosition() + 1);
  buffer.insertCharAtCursor(' ');
}

void TextEditor::deleteRow() {
  if (buffer.isBufferEmpty())
    return;

  size_t rowIndex = buffer.getCurrentRowIndex() - 1;
  size_t rowStart = buffer.getRowStartIndex(rowIndex);
  size_t rowEnd = buffer.getRowEndIndex(rowIndex);
  size_t totalRows = buffer.getRowCount();

  if (totalRows == 1) {
    buffer.clearBuffer();
    buffer.moveGapTo(0);
    status.targetColumn = 0;
    return;
  }

  if (rowIndex == totalRows - 1) {
    buffer.deleteTextInRange(rowStart - 1, rowEnd);
    size_t prevRowStart = buffer.getRowStartIndex(rowIndex - 1);
    buffer.moveGapTo(prevRowStart);
    status.targetColumn = 0;
    return;
  }

  if (rowEnd < buffer.getTextLength() && buffer.getCharAt(rowEnd) == '\n') {
    buffer.deleteTextInRange(rowStart, rowEnd + 1);
  } else {
    buffer.deleteTextInRange(rowStart, rowEnd);
  }

  buffer.moveGapTo(rowStart);
  status.targetColumn = 0;
}

void TextEditor::deleteToEndOfRow() {
  if (buffer.isBufferEmpty())
    return;
  size_t cursorPos = buffer.getCursorPosition();
  size_t rowEnd = buffer.getRowEndIndex(buffer.getCurrentRowIndex() - 1);
  if (rowEnd > cursorPos)
    buffer.deleteTextInRange(cursorPos, rowEnd);
}

void TextEditor::yankRow() {
  size_t rowIndex = buffer.getCurrentRowIndex() - 1;
  yankBuffer = buffer.getRowText(rowIndex);
  if (yankBuffer.empty() || yankBuffer.back() != '\n')
    yankBuffer += '\n';
  status.statusMessage = "Yanked line " + to_string(rowIndex + 1);
}

void TextEditor::yankToEndOfRow() {
  size_t cursorPos = buffer.getCursorPosition();
  size_t rowEnd = buffer.getRowEndIndex(buffer.getCurrentRowIndex() - 1);
  yankBuffer = buffer.getTextInRange(cursorPos, rowEnd);
  if (yankBuffer.empty() || yankBuffer.back() != '\n')
    yankBuffer += '\n';
  status.statusMessage = "Yanked to end of row";
}

void TextEditor::pasteStr(bool after) {
  if (yankBuffer.empty()) {
    status.statusMessage = "Nothing to paste!";
    return;
  }

  if (after) {
    size_t rowEnd = buffer.getRowEndIndex(buffer.getCurrentRowIndex() - 1);
    if (rowEnd == buffer.getTextLength())
      buffer.insertStringAtPos(rowEnd, "\n");

    string yanked = yankBuffer;
    if (!yanked.empty() && yanked.back() == '\n')
      yanked.pop_back();
    buffer.insertStringAtPos(rowEnd + 1, yanked);
    buffer.moveGapTo(rowEnd + 1);
  } else {
    size_t rowStart = buffer.getRowStartIndex(buffer.getCurrentRowIndex() - 1);
    buffer.insertStringAtPos(rowStart, yankBuffer);
    buffer.moveGapTo(rowStart);
  }
}

void TextEditor::indentRow(bool right) {
  size_t rowFirstIndex =
      buffer.getRowStartIndex(buffer.getCurrentRowIndex() - 1);
  if (right) {
    buffer.insertStringAtPos(rowFirstIndex, "    ");
    buffer.moveCursorToRowEnd();
  } else {
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

void TextEditor::performSearch(const string &pattern, bool forward) {
  size_t statusMsgRow =
      0; // Not strictly used by buffer but required by your signature

  // Call the engine
  searchEngine.executeSearch(buffer, pattern, forward, statusMsgRow,
                             status.statusMessage);

  // Update cursor position
  updateStatus(22);
}
void TextEditor::performReplace(const string &from, const string &to,
                                bool all) {
  if (from.empty())
    return;
  size_t rowIdx = buffer.getCurrentRowIndex() - 1;
  string line = buffer.getRowText(rowIdx);
  if (line.empty())
    return;

  size_t pos = line.find(from);
  if (pos == string::npos)
    return;

  UndoGroup::beginGroup();
  int count = 0;
  while (pos != string::npos) {
    size_t startOfRowInBuf = buffer.getRowStartIndex(rowIdx);
    buffer.deleteTextInRange(startOfRowInBuf + pos,
                             startOfRowInBuf + pos + from.length());
    buffer.insertStringAtPos(startOfRowInBuf + pos, to);

    count++;
    if (!all)
      break;

    // Refresh line and search again
    line = buffer.getRowText(rowIdx);
    pos = line.find(from, pos + to.length());
  }
  status.statusMessage = "Replaced " + to_string(count) + " occurrences.";
  fileManager.markAsModified();
}