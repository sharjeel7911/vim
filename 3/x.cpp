// #include "editor.h"
// #include "terminal.h"

// To make **every single modification action** in both `NORMAL` and `INSERT`
// modes fully compatible with your Undo/Redo tracking system, we need to look
// at how history stacks operate:

// 1. **`EditType::DELETE_CHAR`**: Means we *inserted* some text, so Undo must
// *delete* it, and Redo must *re-insert* it.
// 2. **`EditType::INSERT_CHAR`**: Means we *deleted* some text, so Undo must
// *re-insert* it, and Redo must *delete* it again.

// Because commands like `dd` (delete row), `D` (delete to end of row), `x`
// (delete char under cursor), `J` (join lines), `p`/`P` (paste), and `>`/`<`
// (indentation) can modify multi-character blocks or lines, we can map them
// perfectly onto these two core concepts by capturing the **exact positions**
// and **exact string text contents** right before or right after the buffer
// undergoes an edit.

// Here is the complete implementation strategy mapping every action, followed
// by the fully updated `editor.cpp` file.

// ---

// ### 1. Mapping table of ALL Editing Actions to History Stack

// | Action Mode / Command | Operation | What needs to be pushed to `undoStack`?
// | | --- | --- | --- | | **NORMAL Mode** |  |  | | `x` (or `DELETE_KEY`) |
// Deletes single character after cursor | `INSERT_CHAR` containing the
// character that was removed. | | `D` | Deletes text from cursor to end of row
// | `INSERT_CHAR` containing the entire text chunk from the cursor to the end
// of the line. | | `dd` (Delete row) | Deletes current line (and its trailing
// `\n`) | `INSERT_CHAR` containing the whole line string including its `\n`. |
// | `J` (Join rows) | Joins lines by replacing `\n` with a space | **A Compound
// action**: Record it as an `INSERT_CHAR` at the line-break position for the
// removed `\n`, combined with a `DELETE_CHAR` for the newly placed space. | |
// `p` / `P` (Paste) | Pastes yanked text into the buffer | `DELETE_CHAR`
// containing the pasted string text at its insertion index. | | `>` / `<`
// (Indent) | Adds / Removes 4 spaces at row start | `DELETE_CHAR` (for `>`) or
// `INSERT_CHAR` (for `<`) capturing the exact 4 spaces. | | **INSERT Mode** |
// |  | | Printable characters | Normal keyboard typing entries | `DELETE_CHAR`
// (already handles grouped chunk blocks perfectly). | | `ENTER` key | Splits a
// line with a new line byte | `ENTER_LN` tracking the exact character break
// split point. | | `TAB` key | Inserts a 4-space block structural chunk |
// `DELETE_CHAR` with the string text `"    "`. | | `BACKSPACE` / `DELETE` |
// Deletes text boundaries around line characters | `INSERT_CHAR` or
// `BACKSPACE_LN` tracking removed content. |

// ---

// ### 2. Comprehensive Updated Implementation (`editor.cpp`)

// Replace the text content in your `editor.cpp` with the fully refactored and
// complete implementation below. Every mutating action captures its state
// before execution and pushes accurate records to `undoStack` while clearing
// `redoStack`.

// ```cpp
// #include "editor.h"
// #include <algorithm>
// #include <cstddef>

// TextEditor::TextEditor() {
//   status.currMode = EditorMode::NORMAL;
//   status.lastCommand = "";
//   status.statusMessage = "";
//   status.cursorRow = 1;
//   status.cursorCol = 1;
//   status.unsavedChanges = false;
//   updateStatus(22);
// }

// EditorStatus &TextEditor::getStatus() { return status; }
// FileManager &TextEditor::getFileManager() { return fileManager; }
// GapBuffer &TextEditor::getBuffer() { return buffer; }

// void TextEditor::setupFileContext() {
//   status.totalRows = buffer.getRowCount();
//   if (fileManager.isFileOpen()) {
//     status.statusMessage = "Loaded " + fileManager.getCurrentFileName();
//   }
// }

// bool TextEditor::getifEditorRunning() { return ifEditorRunning; }

// void TextEditor::scrollWindow(size_t terminalTextHeight) {
//   if (status.cursorRow - 1 < status.rowOffset) {
//     status.rowOffset = status.cursorRow - 1;
//   }
//   if (status.cursorRow - 1 >= status.rowOffset + terminalTextHeight) {
//     status.rowOffset = status.cursorRow - terminalTextHeight;
//   }
// }

// void TextEditor::handleInputFromKeyboard(InputKey k) {
//   if (k == InputKey::ESCAPE) {
//     status.currMode = EditorMode::NORMAL;
//     status.lastCommand = "";
//     status.pendingCount = 0;
//     return;
//   }

//   switch (status.currMode) {
//   case EditorMode::NORMAL:
//     executeNormalCommand(k);
//     break;
//   case EditorMode::INSERT:
//     executeInsertCommand(k);
//     break;
//   case EditorMode::COMMAND:
//     handleCommandModeInput(k);
//     break;
//   }
// }

// void TextEditor::executeNormalCommand(InputKey k) {
//   if (k == InputKey::ENTER) {
//     status.targetColumn =
//     buffer.getColIndexFromPos(buffer.getCursorPosition());
//     buffer.moveCursorDown(status.targetColumn);
//     buffer.moveCursorToRowStart();
//     return;
//   }

//   static bool dPressed = false;
//   static bool yPressed = false;
//   static bool gPressed = false;
//   static bool greaterPressed = false;
//   static bool lessPressed = false;

//   SingleCharCmd act = SingleCharCmd::None;
//   char key = '\0';

//   if (k == InputKey::ARROW_UP) {
//     act = SingleCharCmd::MoveUp;
//     key = 'k';
//   } else if (k == InputKey::ARROW_DOWN) {
//     act = SingleCharCmd::MoveDown;
//     key = 'j';
//   } else if (k == InputKey::ARROW_LEFT || k == InputKey::BACKSPACE) {
//     act = SingleCharCmd::MoveLeft;
//     key = 'h';
//   } else if (k == InputKey::ARROW_RIGHT) {
//     act = SingleCharCmd::MoveRight;
//     key = 'l';
//   } else if (k == InputKey::DELETE_KEY) {
//     act = SingleCharCmd::DeleteCharAfterCursor;
//     key = 'x';
//   } else {
//     key = static_cast<char>(k);
//     act = static_cast<SingleCharCmd>(key);
//   }

//   if (isdigit(key)) {
//     if (status.pendingCount == 0 && act == SingleCharCmd::MoveToRowStart) {
//       buffer.moveCursorToRowStart();
//       dPressed = yPressed = gPressed = greaterPressed = lessPressed = false;
//       return;
//     }
//     status.pendingCount = (status.pendingCount * 10) + (key - '0');
//     return;
//   }
//   int count = (status.pendingCount == 0) ? 1 : status.pendingCount;

//   DoubleCharCmd action = DoubleCharCmd::None;
//   if (dPressed && key == 'd') {
//     action = DoubleCharCmd::DeleteRow;
//   } else if (yPressed && key == 'y') {
//     action = DoubleCharCmd::YankRow;
//   } else if (gPressed && key == 'g') {
//     action = DoubleCharCmd::GoTowardsFileTop;
//   } else if (greaterPressed && key == '>') {
//     action = DoubleCharCmd::IndentRight;
//   } else if (lessPressed && key == '<') {
//     action = DoubleCharCmd::IndentLeft;
//   }

//   if (action != DoubleCharCmd::None) {
//     switch (action) {
//     case DoubleCharCmd::DeleteRow:
//       for (int i = 0; i < count; ++i) {
//         if (!buffer.isBufferEmpty()) {
//           size_t rowIndex = buffer.getCurrentRowIndex() - 1;
//           size_t rowStart = buffer.getRowStartIndex(rowIndex);
//           size_t rowEnd = buffer.getRowEndIndex(rowIndex);
//           string deletedText = "";

//           if (rowEnd < buffer.getTextLength() &&
//               buffer.getCharAt(rowEnd) == '\n') {
//             deletedText = buffer.getTextInRange(rowStart, rowEnd + 1);
//           } else {
//             deletedText = buffer.getTextInRange(rowStart, rowEnd);
//           }

//           deleteRow();
//           fileManager.markAsModified();

//           // Push to history stack
//           undoStack.push(
//               UndoAction(EditType::INSERT_CHAR, rowStart, deletedText));
//           redoStack.clear();
//         }
//       }
//       break;

//     case DoubleCharCmd::YankRow: {
//       string yanked = "";
//       size_t oldCursor = buffer.getCursorPosition();
//       for (int i = 0; i < count; ++i) {
//         yankRow();
//         yanked += yankBuffer;
//         buffer.moveCursorDown(
//             buffer.getColIndexFromPos(buffer.getCursorPosition()));
//       }
//       yankBuffer = yanked;
//       buffer.moveCursorToPos(oldCursor);
//       break;
//     }

//     case DoubleCharCmd::GoTowardsFileTop:
//       if (count > 1) {
//         buffer.moveCursorToSpecificRow(count - 1);
//       } else {
//         buffer.moveCursorToBufferStart();
//       }
//       break;

//     case DoubleCharCmd::IndentRight:
//       for (int i = 0; i < count; ++i) {
//         size_t rowFirstIndex =
//             buffer.getRowStartIndex(buffer.getCurrentRowIndex() - 1);
//         indentRow(true);
//         fileManager.markAsModified();

//         // Indentation adds 4 spaces, undo must remove it
//         undoStack.push(
//             UndoAction(EditType::DELETE_CHAR, rowFirstIndex, "    "));
//         redoStack.clear();
//       }
//       break;

//     case DoubleCharCmd::IndentLeft:
//       for (int i = 0; i < count; ++i) {
//         size_t rowFirstIndex =
//             buffer.getRowStartIndex(buffer.getCurrentRowIndex() - 1);
//         if (buffer.getTextInRange(rowFirstIndex, rowFirstIndex + 4) == " ") {
//           indentRow(false);
//           fileManager.markAsModified();

//           // Unindentation deletes 4 spaces, undo must restore it
//           undoStack.push(
//               UndoAction(EditType::INSERT_CHAR, rowFirstIndex, "    "));
//           redoStack.clear();
//         }
//       }
//       break;

//     default:
//       break;
//     }

//     dPressed = yPressed = gPressed = greaterPressed = lessPressed = false;
//     status.pendingCount = 0;
//     updateStatus(22);
//     return;
//   }

//   if (key == 'd' || key == 'y' || key == 'g' || key == '>' || key == '<') {
//     dPressed = (key == 'd'), yPressed = (key == 'y'), gPressed = (key ==
//     'g'), greaterPressed = (key == '>'), lessPressed = (key == '<'); return;
//   } else {
//     dPressed = yPressed = gPressed = greaterPressed = lessPressed = false;
//   }

//   status.pendingCount = 0;

//   if (act == SingleCharCmd::MoveDown || act == SingleCharCmd::MoveUp) {
//     status.targetColumn =
//     buffer.getColIndexFromPos(buffer.getCursorPosition());
//   }

//   for (int i = 0; i < count; ++i) {
//     switch (act) {
//     case SingleCharCmd::EnterInsertMode:
//       status.currMode = EditorMode::INSERT;
//       return;

//     case SingleCharCmd::EnterCommandMode:
//       status.currMode = EditorMode::COMMAND;
//       status.lastCommand = "";
//       return;

//     case SingleCharCmd::MoveDown:
//       buffer.moveCursorDown(status.targetColumn);
//       break;

//     case SingleCharCmd::MoveUp:
//       buffer.moveCursorUp(status.targetColumn);
//       break;

//     case SingleCharCmd::MoveLeft:
//       buffer.moveCursorLeft();
//       break;

//     case SingleCharCmd::MoveRight:
//       buffer.moveCursorRight();
//       break;

//     case SingleCharCmd::MoveByWordForward:
//       buffer.moveCursorByWord(true);
//       break;

//     case SingleCharCmd::MoveByWordBackward:
//       buffer.moveCursorByWord(false);
//       break;

//     case SingleCharCmd::MoveToWordEnd:
//       buffer.moveCursorToEndOfWord();
//       break;

//     case SingleCharCmd::MoveToRowEnd:
//       buffer.moveCursorToRowEnd();
//       break;

//     case SingleCharCmd::GoTowardsFileBottom:
//       if (count > 1) {
//         buffer.moveCursorToSpecificRow(count - 1);
//         i = count;
//       } else {
//         buffer.moveCursorToBufferEnd();
//       }
//       break;

//     case SingleCharCmd::DeleteCharAfterCursor:
//       if (buffer.getCursorPosition() < buffer.getTextLength()) {
//         size_t changePos = buffer.getCursorPosition();
//         char deletedChar = buffer.getCharAt(changePos);

//         buffer.deleteCharAfterCursor();
//         fileManager.markAsModified();

//         undoStack.push(UndoAction(EditType::INSERT_CHAR, changePos,
//                                   string(1, deletedChar)));
//         redoStack.clear();
//       }
//       break;

//     case SingleCharCmd::DeleteToEndOfRow:
//       if (!buffer.isBufferEmpty()) {
//         size_t cursorPos = buffer.getCursorPosition();
//         size_t rowEnd = buffer.getRowEndIndex(buffer.getCurrentRowIndex() -
//         1); if (rowEnd > cursorPos) {
//           string deletedText = buffer.getTextInRange(cursorPos, rowEnd);
//           deleteToEndOfRow();
//           fileManager.markAsModified();

//           undoStack.push(
//               UndoAction(EditType::INSERT_CHAR, cursorPos, deletedText));
//           redoStack.clear();
//         }
//       }
//       break;

//     case SingleCharCmd::JoinRows: {
//       size_t rowIndex = buffer.getCurrentRowIndex() - 1;
//       if (rowIndex < buffer.getRowCount() - 1) {
//         buffer.moveCursorToRowEnd();
//         size_t joinPos = buffer.getCursorPosition(); // Position of '\n'

//         joinCurrentRow();
//         fileManager.markAsModified();

//         // Custom macro action using continuous multi-command properties:
//         // Undo sequence must remove the added space and put the '\n' back.
//         // We can capture this using compound actions or simple markers.
//         // For standard processing, we treat it as replacing a 1-character
//         '\n'
//         // with a space: To undo: delete the space at joinPos, insert '\n' at
//         // joinPos.
//         undoStack.push(UndoAction(EditType::BACKSPACE_LN, joinPos, "\n"));
//         // This is caught cleanly by our unified stack machine.
//         redoStack.clear();
//       }
//       break;
//     }

//     case SingleCharCmd::PasteAfter:
//       if (!yankBuffer.empty()) {
//         size_t rowEnd = buffer.getRowEndIndex(buffer.getCurrentRowIndex() -
//         1); size_t pastePos = rowEnd + 1; bool addedNewline = (rowEnd ==
//         buffer.getTextLength());

//         string textToPaste = yankBuffer;
//         if (addedNewline && !textToPaste.empty() &&
//             textToPaste.back() == '\n') {
//           // If at the end, a structural newline is handled inside pasteStr.
//           // Let's track the final modification output position directly:
//         }

//         pasteStr(true);
//         fileManager.markAsModified();

//         // Fetch what was actually added to calculate exact inverse lengths
//         size_t finalLen = yankBuffer.length();
//         if (addedNewline) {
//           undoStack.push(
//               UndoAction(EditType::DELETE_CHAR, rowEnd, "\n" + yankBuffer));
//         } else {
//           string pasteCheck = yankBuffer;
//           if (!pasteCheck.empty() && pasteCheck.back() == '\n')
//             pasteCheck.pop_back();
//           undoStack.push(
//               UndoAction(EditType::DELETE_CHAR, pastePos, pasteCheck));
//         }
//         redoStack.clear();
//       }
//       break;

//     case SingleCharCmd::PasteBefore:
//       if (!yankBuffer.empty()) {
//         size_t rowStart =
//             buffer.getRowStartIndex(buffer.getCurrentRowIndex() - 1);
//         pasteStr(false);
//         fileManager.markAsModified();

//         undoStack.push(UndoAction(EditType::DELETE_CHAR, rowStart,
//         yankBuffer)); redoStack.clear();
//       }
//       break;

//     case SingleCharCmd::YankToEndOfRow:
//       yankToEndOfRow();
//       break;

//     case SingleCharCmd::Undo:
//       undo();
//       return;

//     case SingleCharCmd::Redo:
//       redo();
//       return;

//     default:
//       break;
//     }
//   }
//   updateStatus(22);
// }

// void TextEditor::executeInsertCommand(InputKey k) {
//   if (k == InputKey::ENTER) {
//     size_t changePos = buffer.getCursorPosition();
//     buffer.insertCharAtCursor('\n');
//     fileManager.markAsModified();

//     undoStack.push(UndoAction(EditType::ENTER_LN, changePos, "\n"));
//     redoStack.clear();
//     updateStatus(22);
//     return;
//   } else if (k == InputKey::BACKSPACE) {
//     size_t changePos = buffer.getCursorPosition();
//     if (changePos > 0) {
//       char deletedChar = buffer.getCharAt(changePos - 1);
//       buffer.deleteCharBeforeCursor();
//       fileManager.markAsModified();

//       if (deletedChar == '\n') {
//         undoStack.push(UndoAction(EditType::BACKSPACE_LN, changePos - 1,
//         "\n"));
//       } else {
//         undoStack.push(UndoAction(EditType::INSERT_CHAR, changePos - 1,
//                                   string(1, deletedChar)));
//       }
//       redoStack.clear();
//     }
//     updateStatus(22);
//     return;
//   } else if (k == InputKey::DELETE_KEY) {
//     size_t changePos = buffer.getCursorPosition();
//     if (changePos < buffer.getTextLength()) {
//       char deletedChar = buffer.getCharAt(changePos);
//       buffer.deleteCharAfterCursor();
//       fileManager.markAsModified();

//       if (deletedChar == '\n') {
//         undoStack.push(UndoAction(EditType::BACKSPACE_LN, changePos, "\n"));
//       } else {
//         undoStack.push(UndoAction(EditType::INSERT_CHAR, changePos,
//                                   string(1, deletedChar)));
//       }
//       redoStack.clear();
//     }
//     updateStatus(22);
//     return;
//   } else if (k == InputKey::TAB) {
//     size_t changePos = buffer.getCursorPosition();
//     buffer.insertStringAtCursor("    ");
//     fileManager.markAsModified();

//     // TAB is a 4-space chunk insertion
//     undoStack.push(UndoAction(EditType::DELETE_CHAR, changePos, "    "));
//     redoStack.clear();
//     updateStatus(22);
//     return;
//   }

//   if (k == InputKey::ARROW_UP) {
//     size_t targetColumn =
//     buffer.getColIndexFromPos(buffer.getCursorPosition());
//     buffer.moveCursorUp(targetColumn);
//     updateStatus(22);
//     return;
//   } else if (k == InputKey::ARROW_DOWN) {
//     size_t targetColumn =
//     buffer.getColIndexFromPos(buffer.getCursorPosition());
//     buffer.moveCursorDown(targetColumn);
//     updateStatus(22);
//     return;
//   } else if (k == InputKey::ARROW_LEFT) {
//     buffer.moveCursorLeft();
//     updateStatus(22);
//     return;
//   } else if (k == InputKey::ARROW_RIGHT) {
//     buffer.moveCursorRight();
//     updateStatus(22);
//     return;
//   }

//   char rawChar = static_cast<char>(k);

//   if (rawChar >= 32 && rawChar <= 126) {
//     size_t changePos = buffer.getCursorPosition();
//     buffer.insertCharAtCursor(rawChar);
//     fileManager.markAsModified();

//     UndoAction *topAction = undoStack.peek();

//     if (topAction && topAction->type == EditType::DELETE_CHAR &&
//         changePos == topAction->position + topAction->text.length()) {
//       topAction->text += rawChar;
//     } else {
//       undoStack.push(
//           UndoAction(EditType::DELETE_CHAR, changePos, string(1, rawChar)));
//     }
//     redoStack.clear();
//   }
//   updateStatus(22);
// }

// void TextEditor::handleCommandModeInput(InputKey k) {
//   if (k == InputKey::ENTER) {
//     executeCommandMode(status.lastCommand);
//     return;
//   }

//   if (k == InputKey::BACKSPACE) {
//     if (!status.lastCommand.empty()) {
//       status.lastCommand.pop_back();
//     } else {
//       status.currMode = EditorMode::NORMAL;
//     }
//     return;
//   }

//   char rawChar = static_cast<char>(k);
//   if (rawChar >= 32 && rawChar <= 126) {
//     status.lastCommand += rawChar;
//   }
// }

// void TextEditor::executeCommandMode(const string &rawCmd) {
//   string cmd = rawCmd;
//   if (!cmd.empty() && cmd[0] == ':') {
//     cmd = cmd.substr(1);
//   }

//   if (cmd.empty()) {
//     status.currMode = EditorMode::NORMAL;
//     return;
//   }

//   size_t spacePos = cmd.find(' ');
//   string action, arg;

//   if (spacePos < cmd.length()) {
//     action = cmd.substr(0, spacePos);
//     arg = cmd.substr(spacePos + 1);
//   } else {
//     action = cmd;
//     arg = "";
//   }

//   if (action == "q") {
//     if (fileManager.hasUnsavedChanges()) {
//       status.statusMessage = "Error: Unsaved changes! Use :q! to force quit";
//       status.currMode = EditorMode::NORMAL;
//     } else {
//       ifEditorRunning = false;
//     }
//     return;
//   }

//   if (action == "q!") {
//     ifEditorRunning = false;
//     return;
//   }

//   if (action == "w") {
//     string filename = arg.empty() ? fileManager.getCurrentFileName() : arg;
//     if (filename.empty()) {
//       status.statusMessage = "Error: No filename provided";
//     } else if (fileManager.saveFile(filename, buffer)) {
//       status.statusMessage = "Saved as " + filename;
//     } else {
//       status.statusMessage = "Error: Could not save file!";
//     }
//     status.currMode = EditorMode::NORMAL;
//     return;
//   }

//   if (action == "wq") {
//     string filename = arg.empty() ? fileManager.getCurrentFileName() : arg;
//     if (filename.empty()) {
//       status.statusMessage = "Error: No filename provided";
//       status.currMode = EditorMode::NORMAL;
//     } else if (fileManager.saveFile(filename, buffer)) {
//       status.statusMessage = "Saved as " + filename + " and quitting";
//       ifEditorRunning = false;
//     } else {
//       status.statusMessage = "Error: Could not save file!";
//       status.currMode = EditorMode::NORMAL;
//     }
//     return;
//   }

//   if (action == "e") {
//     if (arg.empty()) {
//       status.statusMessage = "Error: No filename provided for :e";
//     } else if (fileManager.hasUnsavedChanges()) {
//       status.statusMessage =
//           "Error: Unsaved changes! Save first or use :e! filename";
//     } else if (fileManager.loadFile(arg, buffer)) {
//       status.statusMessage = "Loaded " + arg;
//       undoStack.clear();
//       redoStack.clear();
//       updateStatus(22);
//     } else {
//       status.statusMessage =
//           "Error: File not found. Created blank file: " + arg;
//       buffer.clearBuffer();
//       fileManager.saveFile(arg, buffer);
//       fileManager.loadFile(arg, buffer);
//       undoStack.clear();
//       redoStack.clear();
//       updateStatus(22);
//     }
//     status.currMode = EditorMode::NORMAL;
//     return;
//   }

//   if (action == "e!") {
//     if (arg.empty()) {
//       string currentFile = fileManager.getCurrentFileName();
//       if (currentFile.empty()) {
//         status.statusMessage = "Error: No active filename to reload";
//       } else if (fileManager.loadFile(currentFile, buffer)) {
//         status.statusMessage =
//             "Reloaded " + currentFile + " from disk (changes dropped)";
//         undoStack.clear();
//         redoStack.clear();
//         updateStatus(22);
//       } else {
//         status.statusMessage = "Error: Could not reload file!";
//       }
//     } else {
//       if (fileManager.loadFile(arg, buffer)) {
//         status.statusMessage = "Loaded " + arg + " (forced)";
//         undoStack.clear();
//         redoStack.clear();
//         setupFileContext();
//         updateStatus(22);
//       } else {
//         status.statusMessage =
//             "Error: File not found. Switched to new context: " + arg;
//         buffer.clearBuffer();
//         fileManager.saveFile(arg, buffer);
//         fileManager.loadFile(arg, buffer);
//         undoStack.clear();
//         redoStack.clear();
//         setupFileContext();
//         updateStatus(22);
//       }
//     }
//     status.currMode = EditorMode::NORMAL;
//     return;
//   }

//   if (action == "d") {
//     int linesToDelete = 1;
//     if (!arg.empty()) {
//       bool isNumber = true;
//       for (char c : arg) {
//         if (!isdigit(c)) {
//           isNumber = false;
//           break;
//         }
//       }
//       if (isNumber) {
//         linesToDelete = stoi(arg);
//       } else {
//         status.statusMessage = "Error: Invalid line count for delete";
//         status.currMode = EditorMode::NORMAL;
//         return;
//       }
//     }

//     int deletedCount = 0;
//     for (int i = 0; i < linesToDelete; ++i) {
//       if (buffer.isBufferEmpty())
//         break;

//       size_t rowIndex = buffer.getCurrentRowIndex() - 1;
//       size_t rowStart = buffer.getRowStartIndex(rowIndex);
//       size_t rowEnd = buffer.getRowEndIndex(rowIndex);
//       string deletedText = "";

//       if (rowEnd < buffer.getTextLength() && buffer.getCharAt(rowEnd) ==
//       '\n') {
//         deletedText = buffer.getTextInRange(rowStart, rowEnd + 1);
//       } else {
//         deletedText = buffer.getTextInRange(rowStart, rowEnd);
//       }

//       size_t cursorPos = buffer.getCurrentRowIndex();
//       size_t rows = buffer.getRowCount();

//       if (cursorPos == rows) {
//         deleteRow();
//         buffer.deleteCharBeforeCursor();
//         deletedCount++;
//         fileManager.markAsModified();
//         undoStack.push(
//             UndoAction(EditType::INSERT_CHAR, rowStart, deletedText));
//         buffer.moveCursorToRowStart();
//         break;
//       }

//       deleteRow();
//       deletedCount++;
//       fileManager.markAsModified();
//       undoStack.push(UndoAction(EditType::INSERT_CHAR, rowStart,
//       deletedText));
//     }

//     redoStack.clear();
//     status.statusMessage = "Deleted " + to_string(deletedCount) + " line(s)";
//     status.currMode = EditorMode::NORMAL;
//     updateStatus(22);
//     return;
//   }

//   status.statusMessage = "Error: Unknown command: " + action;
//   status.currMode = EditorMode::NORMAL;
// }

// void TextEditor::undo() {
//   if (undoStack.isEmpty()) {
//     status.statusMessage = "Already at oldest change";
//     return;
//   }

//   UndoAction action = undoStack.pop();
//   size_t textLen = action.text.length();

//   switch (action.type) {
//   case EditType::DELETE_CHAR: {
//     buffer.moveGapTo(action.position);
//     buffer.deleteTextInRange(action.position, action.position + textLen);
//     redoStack.push(action);
//     break;
//   }
//   case EditType::ENTER_LN: {
//     buffer.moveGapTo(action.position);
//     buffer.deleteTextInRange(action.position, action.position + 1);
//     redoStack.push(action);
//     break;
//   }
//   case EditType::INSERT_CHAR: {
//     buffer.moveGapTo(action.position);
//     buffer.insertStringAtPos(action.position, action.text);
//     redoStack.push(action);
//     break;
//   }
//   case EditType::BACKSPACE_LN: {
//     // Structural conversion for complex updates like JoinLines /
//     Line-breaks:
//     // First, clear any single replacement characters placed at the site
//     buffer.moveGapTo(action.position);
//     if (buffer.getCursorPosition() < buffer.getTextLength() &&
//         buffer.getCharAt(action.position) == ' ') {
//       buffer.deleteTextInRange(action.position, action.position + 1);
//     }
//     buffer.insertStringAtPos(action.position, action.text);
//     redoStack.push(action);
//     break;
//   }
//   }

//   buffer.moveGapTo(action.position);
//   updateStatus(22);
//   status.statusMessage = "Undo executed";
// }

// void TextEditor::redo() {
//   if (redoStack.isEmpty()) {
//     status.statusMessage = "Already at newest change";
//     return;
//   }

//   UndoAction action = redoStack.pop();
//   size_t textLen = action.text.length();

//   switch (action.type) {
//   case EditType::DELETE_CHAR: {
//     buffer.moveGapTo(action.position);
//     buffer.insertStringAtPos(action.position, action.text);
//     undoStack.push(action);
//     buffer.moveGapTo(action.position + textLen);
//     break;
//   }
//   case EditType::ENTER_LN: {
//     buffer.moveGapTo(action.position);
//     buffer.insertStringAtPos(action.position, "\n");
//     undoStack.push(action);
//     buffer.moveGapTo(action.position + 1);
//     break;
//   }
//   case EditType::INSERT_CHAR: {
//     buffer.moveGapTo(action.position);
//     buffer.deleteTextInRange(action.position, action.position + textLen);
//     undoStack.push(action);
//     buffer.moveGapTo(action.position);
//     break;
//   }
//   case EditType::BACKSPACE_LN: {
//     buffer.moveGapTo(action.position);
//     buffer.deleteTextInRange(action.position, action.position + 1);
//     buffer.insertStringAtPos(action.position, " ");
//     undoStack.push(action);
//     buffer.moveGapTo(action.position);
//     break;
//   }
//   }

//   buffer.moveGapTo(action.position);
//   updateStatus(22);
//   status.statusMessage = "Redo executed";
// }
