#include "searchengine.h"

SearchEngine::SearchEngine() : lastSearchPattern(""), caseInsensitive(true) {}

string SearchEngine::toLower(const string& str) {
  string lowerStr = str;
  for (char& c : lowerStr) {
    c = tolower(c);
  }
  return lowerStr;
}

string SearchEngine::getLastSearchPattern() { return lastSearchPattern; }

void SearchEngine::executeSearch(GapBuffer& buffer, const string& pattern, bool forward, size_t& statusMsgRow, string& statusMsg) {
  if (pattern.empty()) return;
  lastSearchPattern = pattern;

  size_t currentPos = buffer.getCursorPosition();
  size_t textLen = buffer.getTextLength();
  size_t pLen = pattern.length();

  size_t matchIndex = (size_t)-1;

  if (forward) {
    // search forward from current position + 1
    matchIndex = buffer.findNextPatternFirstIndex(pattern.c_str(), currentPos + 1);

    // if not found, wrap around to the start 
    if (matchIndex == (size_t)-1) {
      matchIndex = buffer.findNextPatternFirstIndex(pattern.c_str(), 0);
    }
  }
  else {
    // backward search
    matchIndex = buffer.findPreviousPatternFirstIndex(pattern.c_str(), currentPos);

    // if not found, wrap around to the end
    if (matchIndex == (size_t)-1) {
      matchIndex = buffer.findPreviousPatternFirstIndex(pattern.c_str(), textLen - 1);
    }
  }

  if (matchIndex != (size_t)-1) {
    buffer.moveCursorToPos(matchIndex);
    statusMsg = "Found: " + pattern;
  }
  else {
    statusMsg = "Pattern not found: " + pattern;
  }
}

void SearchEngine::findNext(GapBuffer& buffer, size_t& statusMsgRow, string& statusMsg) {
  executeSearch(buffer, lastSearchPattern, true, statusMsgRow, statusMsg);
}

void SearchEngine::findPrev(GapBuffer& buffer, size_t& statusMsgRow, string& statusMsg) {
  executeSearch(buffer, lastSearchPattern, false, statusMsgRow, statusMsg);
}

void SearchEngine::executeSubstitution(GapBuffer& buffer, const string& commandStr, string& statusMsg) {
  // expected incoming string formats: "s/old/new" or "s/old/new/g"
  if (commandStr.length() < 5 || commandStr[0] != 's' || commandStr[1] != '/') {
    statusMsg = "Invalid substitution syntax. Use :s/old/new/g";
    return;
  }

  vector<string> tokens;
  string currentToken = "";
  for (size_t i = 2; i < commandStr.length(); ++i) {
    if (commandStr[i] == '/') {
      tokens.push_back(currentToken);
      currentToken = "";
    }
    else {
      currentToken += commandStr[i];
    }
  }
  tokens.push_back(currentToken);

  if (tokens.size() < 2) {
    statusMsg = "Invalid syntax. Missing replacement string.";
    return;
  }

  string oldStr = tokens[0];
  string newStr = tokens[1];
  bool global = (tokens.size() > 2 && tokens[2] == "g");

  if (oldStr.empty()) {
    statusMsg = "Error: Cannot replace an empty pattern.";
    return;
  }

  size_t currentRow = buffer.getCurrentRowIndex() - 1;
  size_t rowStart = buffer.getRowStartIndex(currentRow);
  size_t rowEnd = buffer.getRowEndIndex(currentRow);

  // safely extract raw line content once
  string lineText = "";
  for (size_t i = rowStart; i < rowEnd; ++i) {
    lineText += buffer.getCharAt(i);
  }

  // perform replacements purely inside the local string instance
  size_t pos = lineText.find(oldStr);
  if (pos == string::npos) {
    statusMsg = "Pattern not found on current line.";
    return;
  }

  size_t replaceCount = 0;
  while (pos != string::npos) {
    lineText.replace(pos, oldStr.length(), newStr);
    replaceCount++;

    if (!global) break;
    pos = lineText.find(oldStr, pos + newStr.length());
  }
  buffer.deleteTextInRange(rowStart, rowEnd);
  buffer.insertStringAtPos(rowStart, lineText);
  statusMsg = "Replaced " + to_string(replaceCount) + " occurrence(s).";
}