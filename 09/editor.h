#include "filemanager.h"
#include "gapbuffer.h"
#include "history.h"
#include "searchengine.h"

#ifndef EDITOR_H
#define EDITOR_H

// ----------------------------------------------------------------------------------------------------
// GLobal ENUMS
// ----------------------------------------------------------------------------------------------------

enum class EditorMode { NORMAL, INSERT, COMMAND };

// for getting one input key from terminal / keyboard
enum class InputKey {
  UNKNOWN,

  TAB,
  ENTER,
  ESCAPE,
  BACKSPACE,
  DELETE_KEY,

  ARROW_UP,
  ARROW_DOWN,
  ARROW_LEFT,
  ARROW_RIGHT
};

// for 2 char long command in NORMAL mode
enum class DoubleCharCmd {
  None,

  YankRow,
  DeleteRow,
  IndentLeft,
  IndentRight,
  GoTowardsFileTop
};

// for 1 char long command in NORMAL mode
enum class SingleCharCmd {
  None,

  Undo = 'u',
  Redo = 18,
  SearchFile = '/',
  NextMatch = 'n',
  PrevMatch = 'N',

  EnterInsertMode = 'i',
  EnterCommandMode = ':',

  // cursor motions
  MoveUp = 'k',
  MoveDown = 'j',
  MoveLeft = 'h',
  MoveRight = 'l',

  MoveToRowEnd = '$',
  MoveToRowStart = '0',
  MoveToWordEnd = 'e',
  MoveByWordForward = 'w',
  MoveByWordBackward = 'b',
  GoTowardsFileBottom = 'G',

  // edit actions
  PasteAfter = 'p',
  PasteBefore = 'P',
  JoinRows = 'J',
  YankToEndOfRow = 'Y',
  DeleteToEndOfRow = 'D',
  DeleteCharAfterCursor = 'x',
};

// ----------------------------------------------------------------------------------------------------
// Terminal & Status
// ----------------------------------------------------------------------------------------------------

// foward declaration
class Terminal;

// struct for status bar
struct EditorStatus {
  EditorMode currMode = EditorMode::NORMAL;
  string lastCommand = "";
  string statusMessage = "";

  size_t cursorRow;
  size_t cursorCol;
  size_t rowOffset = 0;

  size_t totalRows = 0;
  size_t targetColumn = 0; // remembers where the user "wants" to be

  int pendingCount = 0;
  bool unsavedChanges = false;
};

// ----------------------------------------------------------------------------------------------------
// TextEditor class
// ----------------------------------------------------------------------------------------------------

#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H
class TextEditor {
private:
  EditorStatus status;
  GapBuffer buffer;
  FileManager fileManager;
  SearchEngine searchEngine;

  HistoryStack undoStack;
  HistoryStack redoStack;

  string yankBuffer;
  bool ifEditorRunning = true;

  friend class Terminal;
public:
  TextEditor();
  // void run(const string &);

  EditorStatus& getStatus();
  FileManager& getFileManager();
  GapBuffer& getBuffer();
  void setupFileContext();

  // returns true if the program still running
  bool getifEditorRunning();

  void scrollWindow(size_t);

  void undo();
  void redo();

  // gets a key from terminal then send it to appropriate func to act upon it
  void handleInputFromKeyboard(InputKey);
  void executeNormalCommand(InputKey);
  void executeInsertCommand(InputKey);
  void handleCommandModeInput(InputKey);
  void executeCommandMode(const string&);

  void deleteRow();
  void deleteToEndOfRow();
  void yankRow();
  void yankToEndOfRow();
  void joinCurrentRow();
  void pasteStr(bool);
  void indentRow(bool);
  void updateStatus(size_t);

  // file operation methods
  void updateModifiedStatus();
  bool attemptQuit();

  // file/search/replace 
  void performSearch(const string&, bool);
  void performReplace(const string&, const string&, bool);
};
#endif
#endif