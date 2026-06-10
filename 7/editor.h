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
  // constructor
  TextEditor();
  // main program loop
  // void run(const string &);

  EditorStatus &getStatus();
  FileManager &getFileManager();
  GapBuffer &getBuffer();
  void setupFileContext();

  // returns true if the program still running
  bool getifEditorRunning();

  void scrollWindow(size_t);

  // 3. Declare the core timeline handlers stack
  void undo();
  void redo();

  // gets a key from terminal then send it to appropriate func to act upon it
  void handleInputFromKeyboard(InputKey);
  void executeNormalCommand(InputKey);
  void executeInsertCommand(InputKey);
  void handleCommandModeInput(InputKey);
  void executeCommandMode(const string &);

  // gets input from handleInput() in 1 or 2 char and runs it in Normal mode
  void executeNormalCommand(char);
  void executeInsertCommand(char);
  // CHANGED: Improved command mode handling
  // void executeCommandMode(const string &);

  // Vim Functionality (The "Policy" layer)
  void deleteRow();
  void deleteToEndOfRow();
  void yankRow();
  void yankToEndOfRow();
  void joinCurrentRow();
  void pasteStr(bool);
  void indentRow(bool);
  void updateStatus(size_t);

  // Added file operation methods
  void updateModifiedStatus();
  bool attemptQuit();

  // File/Search/Replace (The Brains)
  void performSearch(const string &, bool);
  void performReplace(const string &, const string &, bool);
};

#endif
#endif

//----------------------------------------------

/*
namespace Colors {

const string Reset = "\033[0m";
const string Bold = "\033[1m";

const string Black = "\033[30m";
const string Red = "\033[31m";
const string Green = "\033[32m";
const string Yellow = "\033[33m";
const string Blue = "\033[34m";
const string Magenta = "\033[35m";
const string Cyan = "\033[36m";
const string White = "\033[37m";

const string BrightRed = "\033[91m";
const string BrightGreen = "\033[92m";
const string BrightYellow = "\033[93m";
const string BrightBlue = "\033[94m";}

namespace Colors
{
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define DIM "\033[2m"
#define UNDERLINE "\033[4m"

// Foreground colors
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define BRIGHT_RED "\033[91m"
#define BRIGHT_GREEN "\033[92m"
#define BRIGHT_YELLOW "\033[93m"
#define BRIGHT_BLUE "\033[94m"
#define BRIGHT_MAGENTA "\033[95m"
#define BRIGHT_CYAN "\033[96m"
#define BRIGHT_WHITE "\033[97m"

// Background colors
#define BG_BLACK "\033[40m"
#define BG_BLUE "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN "\033[46m"
#define BG_WHITE "\033[47m"
#define BG_BRIGHT_BLUE "\033[104m"
#define BG_BRIGHT_CYAN "\033[106m"
}

void clearScreen() { cout << "\033[2J\033[H"; }

*/