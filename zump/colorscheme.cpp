#include "colorscheme.h"

ColorScheme::ColorScheme() : activeLanguage("plaintext") {
  initializeCppKeywords();
  initializePythonKeywords();
  initializeJsKeywords();
}

// CHANGED: Initialize C++ keywords
void ColorScheme::initializeCppKeywords() {
  vector<string> keywords = {"if",        "else",      "for",    "while",   "do",    "switch", "case",    "break",
                             "return",    "class",     "struct", "enum",    "void",  "int",    "char",    "float",
                             "double",    "bool",      "string", "auto",    "const", "static", "private", "public",
                             "protected", "namespace", "using",  "include", "define"};
  for (const auto& kw : keywords) cppKeywords[kw] = TokenType::KEYWORD;
}

// CHANGED: Initialize Python keywords
void ColorScheme::initializePythonKeywords() {
  vector<string> keywords = {"if",   "else",   "elif", "for",   "while",  "def",     "class", "return", "import",
                             "from", "as",     "True", "False", "None",   "and",     "or",    "not",    "in",
                             "is",   "lambda", "with", "try",   "except", "finally", "raise"};
  for (const auto& kw : keywords) pythonKeywords[kw] = TokenType::KEYWORD;
}

// CHANGED: Initialize JavaScript keywords
void ColorScheme::initializeJsKeywords() {
  vector<string> keywords = {"if",     "else", "for",   "while",   "do",    "switch", "case",   "function",
                             "return", "var",  "let",   "const",   "class", "new",    "this",   "async",
                             "await",  "try",  "catch", "finally", "throw", "import", "export", "default"};
  for (const auto& kw : keywords) jsKeywords[kw] = TokenType::KEYWORD;
}

// CHANGED: Set active language from file extension
void ColorScheme::setLanguageFromFile(const string& filename) {
  if (filename.empty()) return;

  size_t dotPos = filename.rfind('.');
  if (dotPos == string::npos) {
    activeLanguage = "plaintext";
    return;
  }

  string ext = filename.substr(dotPos + 1);
  // Convert to lowercase
  for (char& c : ext) c = std::tolower(c);

  if (ext == "cpp" || ext == "cc" || ext == "h" || ext == "hpp") {
    activeLanguage = "cpp";
  } else if (ext == "py") {
    activeLanguage = "python";
  } else if (ext == "js" || ext == "jsx" || ext == "ts" || ext == "tsx") {
    activeLanguage = "javascript";
  } else {
    activeLanguage = "plaintext";
  }
}

// CHANGED: Get token type for a word based on active language
TokenType ColorScheme::getTokenType(const string& word) {
  if (word.empty()) return TokenType::NORMAL;

  if (activeLanguage == "cpp") {
    auto it = cppKeywords.find(word);
    if (it != cppKeywords.end()) return it->second;
  } else if (activeLanguage == "python") {
    auto it = pythonKeywords.find(word);
    if (it != pythonKeywords.end()) return it->second;
  } else if (activeLanguage == "javascript") {
    auto it = jsKeywords.find(word);
    if (it != jsKeywords.end()) return it->second;
  }

  return TokenType::NORMAL;
}

// CHANGED: Convert token type to ANSI color code
string ColorScheme::getColorForToken(TokenType type) {
  switch (type) {
    case TokenType::KEYWORD:
      return "\x1b[38;5;208m";  // Orange for keywords
    case TokenType::FUNCTION:
      return "\x1b[38;5;81m";  // Cyan for functions
    case TokenType::STRING:
      return "\x1b[38;5;78m";  // Green for strings
    case TokenType::COMMENT:
      return "\x1b[38;5;244m";  // Gray for comments
    case TokenType::NUMBER:
      return "\x1b[38;5;135m";  // Magenta for numbers
    case TokenType::OPERATOR:
      return "\x1b[38;5;215m";  // Light orange for operators
    case TokenType::PUNCTUATION:
      return "\x1b[38;5;244m";  // Gray for punctuation
    default:
      return reset();  // Normal text
  }
}

// +----------------------------------+
// ANSI Sequence Implementations
// +----------------------------------+

// CHANGED: Forced background color (deep dark blue-gray, overrides terminal)
string ColorScheme::bgColor() { return "\x1b[48;5;235m"; }

// CHANGED: Status bar background (slightly lighter than main)
string ColorScheme::bgStatusBar() { return "\x1b[48;5;238m"; }

// CHANGED: Search highlight background (bright cyan)
string ColorScheme::bgHighlight() { return "\x1b[48;5;51m"; }

// CHANGED: Normal text foreground (bright white)
string ColorScheme::fgNormal() { return "\x1b[38;5;255m"; }

// CHANGED: Muted text foreground (medium gray)
string ColorScheme::fgMuted() { return "\x1b[38;5;244m"; }

// CHANGED: Reset all formatting
string ColorScheme::reset() { return "\x1b[0m"; }

// CHANGED: Move cursor (row, col)
string ColorScheme::moveCursor(size_t row, size_t col) { return "\x1b[" + to_string(row) + ";" + to_string(col) + "H"; }

// CHANGED: Hide cursor
string ColorScheme::hideCursor() { return "\x1b[?25l"; }

// CHANGED: Show cursor
string ColorScheme::showCursor() { return "\x1b[?25h"; }

// CHANGED: Clear screen
string ColorScheme::clearScreen() { return "\x1b[2J"; }

// CHANGED: Clear line
string ColorScheme::clearLine() { return "\x1b[K"; }

// CHANGED: Alt buffer on
string ColorScheme::altBufferOn() { return "\x1b[?1049h"; }

// CHANGED: Alt buffer off
string ColorScheme::altBufferOff() { return "\x1b[?1049l"; }

// CHANGED: NORMAL mode badge color (blue background + white text)
string ColorScheme::modeNormal() { return "\x1b[48;5;33m\x1b[38;5;232m\x1b[1m"; }

// CHANGED: INSERT mode badge color (green background + dark text)
string ColorScheme::modeInsert() { return "\x1b[48;5;71m\x1b[38;5;232m\x1b[1m"; }

// CHANGED: COMMAND mode badge color (red background + white text)
string ColorScheme::modeCommand() { return "\x1b[48;5;196m\x1b[38;5;255m\x1b[1m"; }

// CHANGED: Italic text
string ColorScheme::italic() { return "\x1b[3m"; }