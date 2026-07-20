<div align="center">

# ⌨️ Shar

A fully functional **Vim-inspired terminal text editor** built in **C++** using **Gap Buffer**

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Data Structures](https://img.shields.io/badge/Data%20Structures-Gap%20Buffer-green?style=for-the-badge)
![Modal Editing](https://img.shields.io/badge/Modal%20Editing-Vim%20Style-purple?style=for-the-badge)
![Terminal](https://img.shields.io/badge/Terminal-Raw%20Mode-black?style=for-the-badge)

</div>

---

## 📋 About

**Shar** is a lightweight, modal text editor running directly in your terminal — inspired by Vim's power and simplicity. Built entirely from scratch using **C++17**, it demonstrates advanced data structures (Gap Buffer), efficient text manipulation, and terminal handling at the systems level.

The editor implements three distinct modes (Normal, Insert, Command), a custom undo/redo system with grouped actions, and comprehensive file management — all without external UI libraries.

> 📌 This is my **Final Project** for the **Data Structures** course.

---

## 🎯 Core Features

### ✏️ **Modal Editing**

- **Normal Mode** — navigate, delete, yank, paste, and command composition
- **Insert Mode** — type text with full undo support and smart character batching
- **Command Mode** — execute file operations, search, and substitutions

### 🚀 **Text Manipulation**

- **Gap Buffer** — O(1) insertions at cursor, efficient memory management
- **Undo/Redo** — grouped actions for multi-step commands (e.g., `5>>` is one undo)
- **Word Navigation** — `w`, `b`, `e` for word-level movement
- **Line Operations** — yank, delete, indent, join rows
- **Text Substitution** — `:s/old/new` and `:s/old/new/g` with proper undo integration

### 🔍 **Search & Highlighting**

- **Forward/Backward Search** — `/pattern` with case-insensitive matching
- **Match Navigation** — `n` (next), `N` (previous) to jump through results
- **Live Highlighting** — search matches highlighted in cyan during display
- **Pattern Wrapping** — searches wrap around file boundaries

### 🎨 **Syntax Highlighting** ⚠️ _In Progress_

- Auto-detection by file extension (`.cpp`, `.py`, `.js`)
- Keyword-based coloring for C++, Python, JavaScript
- Extensible `ColorScheme` class ready for additional languages
- _Status_: WORK IN PROGRESS (IN FREE TIME SOON)

### 💾 **File Management**

- **Load/Save** — `:w`, `:e filename`, `:wq`, `:q`, `:q!`
- **Unsaved Change Tracking** — modified indicator in status bar
- **Blank File Creation** — auto-create new files on `:e newfile.txt`
- **Current File Display** — status bar shows active filename

### 📊 **Status Bar & UI**

- **Real-time Cursor Position** — line and column numbers
- **Mode Indicator** — color-coded (green=INSERT, red=COMMAND, blue=NORMAL)
- **Total Line Count** — for quick file size reference
- **Status Messages** — search results, error states, operation feedback
- **Command Echo** — live display of `:command` as you type

---

## 🕹️ Command Reference

### **Normal Mode Commands**

#### **Movement**

| Command  | Action                      |
| -------- | --------------------------- |
| `h`, `←` | Move left                   |
| `j`, `↓` | Move down                   |
| `k`, `↑` | Move up                     |
| `l`, `→` | Move right                  |
| `w`      | Jump to next word start     |
| `b`      | Jump to previous word start |
| `e`      | Jump to word end            |
| `0`      | Jump to line start          |
| `$`      | Jump to line end            |
| `gg`     | Jump to file start          |
| `G`      | Jump to file end            |
| `:110`   | Jump to line 110            |

#### **Editing**

| Command      | Action                                                         |
| ------------ | -------------------------------------------------------------- |
| `i`          | Enter Insert Mode                                              |
| `x`          | Delete character at cursor                                     |
| `d` + motion | Delete text (e.g., `dw` = delete word, `5dd` = delete 5 lines) |
| `dd`         | Delete entire line                                             |
| `D`          | Delete to end of line                                          |
| `y` + motion | Yank (copy) text                                               |
| `yy`         | Yank entire line                                               |
| `Y`          | Yank to end of line                                            |
| `p`          | Paste after cursor                                             |
| `P`          | Paste before cursor                                            |
| `J`          | Join current line with next line                               |

#### **Indentation**

| Command | Action                               |
| ------- | ------------------------------------ |
| `>>`    | Indent line right (add 4 spaces)     |
| `<<`    | Unindent line left (remove 4 spaces) |
| `5>>`   | Indent 5 lines right                 |
| `3<<`   | Unindent 3 lines left                |

#### **Search & Navigation**

| Command    | Action                     |
| ---------- | -------------------------- |
| `/pattern` | Search forward for pattern |
| `n`        | Jump to next match         |
| `N`        | Jump to previous match     |

#### **Undo/Redo**

| Command  | Action                     |
| -------- | -------------------------- |
| `u`      | Undo last change(s)        |
| `Ctrl+R` | Redo last undone change(s) |

#### **Count Prefix**

Commands can be prefixed with counts:

- `5j` — move down 5 lines
- `3dd` — delete 3 lines
- `10x` — delete 10 characters
- `5>>` — indent 5 lines (undone as one action)

---

### **Insert Mode Commands**

| Command            | Action                         |
| ------------------ | ------------------------------ |
| `Esc`              | Exit to Normal Mode            |
| `Enter`            | Insert newline                 |
| `Backspace`        | Delete character before cursor |
| `Delete`           | Delete character at cursor     |
| `Tab`              | Insert 4 spaces (soft tabs)    |
| `Arrow Keys`       | Navigate while in Insert Mode  |
| Any printable char | Insert character at cursor     |

---

### **Command Mode (`:` commands)**

#### **File Operations**

| Command        | Action                       |
| -------------- | ---------------------------- |
| `:w`           | Save current file            |
| `:w filename`  | Save as filename             |
| `:e filename`  | Open/load filename           |
| `:e! filename` | Force open (discard changes) |
| `:q`           | Quit (error if unsaved)      |
| `:q!`          | Force quit (discard changes) |
| `:wq`          | Save and quit                |

#### **Text Substitution**

| Command        | Action                                   |
| -------------- | ---------------------------------------- |
| `:s/old/new`   | Replace first occurrence on current line |
| `:s/old/new/g` | Replace all occurrences on current line  |

#### **Deletion** ⚠️ _Command Mode Only_

| Command | Action                               |
| ------- | ------------------------------------ |
| `:d`    | Delete current line                  |
| `:d 5`  | Delete 5 lines from current position |

---

### **Escape Key Behavior**

- **Normal Mode** → stays in Normal Mode (no-op)
- **Insert Mode** → exit to Normal Mode
- **Command Mode** → cancel command, return to Normal Mode

---

## 📊 Data Structures Used

### **Gap Buffer** (Core Storage)

- Efficient O(1) insertions at cursor position
- Minimizes memory copies during editing
- Maintains contiguous text on both sides of the gap
- Automatic growth when gap fills

### **Linked List Stack** (Undo/Redo)

- Custom `HistoryStack` with `UndoNode*` pointers
- Groups related actions by `groupId` for atomic undo
- Example: `5>>` (5 indent operations) undone as one step
- Separate stacks for undo and redo

---

## 🚀 How to Run

### **Prerequisites**

- C++17 compiler (`g++`, `clang`, or MSVC)
- Linux/macOS/WSL/UNIX terminal (Windows native support limited)

### **Build & Run**

```bash
# Clone or navigate to project directory in terminal
cd shar

# Compile with provided Makefile
make

# Run the editor
./shar                        # open blank editor
./shar <filename>.<ext>       # open or create any file
```

### **Installation (Optional)**

```bash
# Install globally to /usr/local/bin
make install

# Now use from anywhere
shar <filename>.<ext>
```

---

## ⚠️ Known Limitations & In-Progress Features

### **Syntax Highlighting** 🔧 _Framework Complete_

- ✅ Auto-detection by file extension
- ✅ Keyword maps for C++, Python, JavaScript
- ⏳ Full rendering integration pending
- **Status**: Coloring logic exists but needs working on

### **Line Wrapping** 🔧 _Partial_

- ✅ Long lines wrap visually on screen
- ⏳ Cursor positioning in wrapped segments needs refinement
- **Status**: Functional but may have edge cases

### **Bracketed Paste Mode** 🔧 _Under Investigation_

- Terminal may not send paste escape sequences correctly
- Raw characters flood stdin in Normal Mode
- **Workaround**: Use Insert Mode for multi-line pastes
- **Status**: Diagnostic test in progress

---

## 🐛 Debugging Notes

### **Terminal Mode Issues**

If the editor doesn't respond to keys or display is corrupted:

1. Press `Ctrl+C` to force exit
2. Type `reset` to restore terminal
3. Verify terminal emulator supports raw mode

### **Cursor Position Bugs** (Fixed)

- Previous issue: `size_t` underflow causing huge scrolls
- **Solution**: Use `static_cast<int>` and explicit bounds checking in `scrollWindow()`

### **Search Case Sensitivity** (Fixed)

- Previous issue: Highlighting and movement used different matching logic
- **Solution**: Unified to case-insensitive in both paths

### **Clangd False Positives**

- Mutual includes between `terminal.h` and `editor.h` cause ~20 spurious errors
- **Status**: Real bugs are zero — confirmed by successful execution
- **Workaround**: Ignore clangd diagnostics; trust compilation

---

## 📁 Project Structure

```
Shar/
├── src/
│   ├── main.cpp                      			 # Entry point
│   ├── editor.cpp / editor.h         			 # Core editor state & commands
│   ├── gapbuffer.cpp / gapbuffer.h   			 # Gap buffer implementation
│   ├── terminal.cpp / terminal.h     			 # Raw terminal & rendering
│   ├── searchengine.cpp / searchengine.h 	 # Search & substitution
│   ├── filemanager.cpp / filemanager.h   	 # File I/O & state
│   ├── history.h                     			 # Undo/redo data structures
│   ├── utilities.h                   		   # ANSI sequences & helpers
├── makefile                        		     # Build configuration
├── README.md
└── LICENSE
```

---

## 🎓 Learning Outcomes

This project demonstrates:

- **Data Structures**: Gap Buffer for efficient text storage, Stack for undo/redo
- **Systems Programming**: Raw terminal mode, signal handling, ANSI escape sequences
- **Software Architecture**: Modal design patterns, separation of concerns
- **C++ Best Practices**: Smart memory management, RAII, const-correctness
- **Algorithm Design**: O(1) insertions, pattern matching, cursor movement

---

## 📄 License

This project is open source and available under the [MIT License](https://github.com/Sharjeel7911/text-editor/blob/main/LICENSE).

---

<div align="center">

Made with ❤️ and C++

</div>
