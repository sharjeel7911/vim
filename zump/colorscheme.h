#ifndef COLORSCHEME_H
#define COLORSCHEME_H

#include "utilities.h"

// Token types for language-specific syntax highlighting
enum class TokenType {
  NORMAL,       // Regular text
  KEYWORD,      // Language keywords (if, for, class, etc.)
  FUNCTION,     // Function/method names
  STRING,       // String literals
  COMMENT,      // Comments
  NUMBER,       // Numeric literals
  OPERATOR,     // Operators (+, -, *, etc.)
  PUNCTUATION,  // Brackets, semicolons, etc.
};

class ColorScheme {
 private:
  // CHANGED: Keyword maps for different languages
  unordered_map<string, TokenType> cppKeywords;
  unordered_map<string, TokenType> pythonKeywords;
  unordered_map<string, TokenType> jsKeywords;

  // CHANGED: Active language (default: plaintext)
  string activeLanguage;

  void initializeCppKeywords();
  void initializePythonKeywords();
  void initializeJsKeywords();

 public:
  ColorScheme();

  // CHANGED: Set active language based on file extension
  void setLanguageFromFile(const string& filename);

  // CHANGED: Get token type for a word (for syntax highlighting)
  TokenType getTokenType(const string& word);

  // CHANGED: Convert token type to ANSI color code
  string getColorForToken(TokenType type);

  // +----------------------------------+
  // ANSI Sequence Helpers
  // +----------------------------------+

  // CHANGED: Forced persistent background (will override terminal defaults)
  static string bgColor();      // Main editor background (dark)
  static string bgStatusBar();  // Status bar background
  static string bgHighlight();  // Search highlight background
  static string fgNormal();     // Normal text color
  static string fgMuted();      // Dimmed/muted text
  static string reset();        // Reset all formatting
  static string moveCursor(size_t row, size_t col);
  static string hideCursor();
  static string showCursor();
  static string clearScreen();
  static string clearLine();
  static string altBufferOn();
  static string altBufferOff();

  // Mode indicators
  static string modeNormal();   // NORMAL mode badge color
  static string modeInsert();   // INSERT mode badge color
  static string modeCommand();  // COMMAND mode badge color

  // CHANGED: Italic text (for status messages)
  static string italic();
};

#endif