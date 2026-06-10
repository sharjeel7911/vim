#include "searchengine.h"

SearchEngine::SearchEngine() : lastSearchPattern(""), caseInsensitive(true) {}

string SearchEngine::toLower(const string &str) {
  string lowerStr = str;
  for (char &c : lowerStr)
    c = std::tolower(c);
  return lowerStr;
}

// Global forward/backward buffer scanning
void SearchEngine::executeSearch(GapBuffer &buffer, const string &pattern,
                                 bool forward, size_t &statusMsgRow,
                                 string &statusMsg) {
  if (pattern.empty())
    return;
  lastSearchPattern = pattern;

  size_t currentPos = buffer.getCursorPosition();
  size_t textLen = buffer.getTextLength();
  size_t pLen = pattern.length();

  size_t matchIndex = (size_t)-1;

  if (forward) {
    // Search forward from current position + 1
    matchIndex =
        buffer.findNextPatternFirstIndex(pattern.c_str(), currentPos + 1);

    // If not found, wrap around to the start (Classic Vim behavior)
    if (matchIndex == (size_t)-1) {
      matchIndex = buffer.findNextPatternFirstIndex(pattern.c_str(), 0);
    }
  } else {
    // Backward search
    matchIndex =
        buffer.findPreviousPatternFirstIndex(pattern.c_str(), currentPos);

    // If not found, wrap around to the end
    if (matchIndex == (size_t)-1) {
      matchIndex =
          buffer.findPreviousPatternFirstIndex(pattern.c_str(), textLen - 1);
    }
  }

  if (matchIndex != (size_t)-1) {
    buffer.moveCursorToPos(matchIndex);
    statusMsg = "Found: " + pattern;
  } else {
    statusMsg = "Pattern not found: " + pattern;
  }
}

void SearchEngine::findNext(GapBuffer &buffer, size_t &statusMsgRow,
                            string &statusMsg) {
  executeSearch(buffer, lastSearchPattern, true, statusMsgRow, statusMsg);
}

void SearchEngine::findPrev(GapBuffer &buffer, size_t &statusMsgRow,
                            string &statusMsg) {
  executeSearch(buffer, lastSearchPattern, false, statusMsgRow, statusMsg);
}

void SearchEngine::executeSubstitution(GapBuffer &buffer,
                                       const string &commandStr,
                                       string &statusMsg) {
  // Expected incoming string formats: "s/old/new" or "s/old/new/g"
  if (commandStr.length() < 5 || commandStr[0] != 's' || commandStr[1] != '/') {
    statusMsg = "Invalid substitution syntax. Use :s/old/new/g";
    return;
  }

  // Parse out tokens split by '/'
  vector<string> tokens;
  stringstream ss(commandStr.substr(2)); // Skip 's/'
  string token;
  while (getline(ss, token, '/')) {
    tokens.push_back(token);
  }

  if (tokens.size() < 2) {
    statusMsg = "Invalid syntax. Missing replacement string.";
    return;
  }

  string oldStr = tokens[0];
  string newStr = tokens[1];
  bool global = (tokens.size() > 2 && tokens[2] == "g");

  // Grab current row start and end boundary indices from the GapBuffer
  size_t currentRow = buffer.getCurrentRowIndex() - 1;
  size_t rowStart = buffer.getRowStartIndex(currentRow);
  size_t rowEnd = buffer.getRowEndIndex(currentRow);

  // Extract raw line content
  string lineText = "";
  for (size_t i = rowStart; i < rowEnd; ++i) {
    lineText += buffer.getCharAt(i);
  }

  // Process replacement configurations
  size_t pos = lineText.find(oldStr);
  if (pos == string::npos) {
    statusMsg = "Pattern not found on current line.";
    return;
  }

  size_t replaceCount = 0;
  while (pos != string::npos) {
    // Delete old string and insert new string into the gap buffer
    size_t absolutePos = rowStart + pos;
    buffer.deleteTextInRange(absolutePos, absolutePos + oldStr.length());
    buffer.insertStringAtPos(absolutePos, newStr);

    replaceCount++;
    if (!global)
      break; // If no '/g' flag, stop after replacing first instance

    // Recalculate row lengths dynamically since modifications shift buffer
    // sizes
    rowEnd = buffer.getRowEndIndex(currentRow);
    lineText = "";
    for (size_t i = rowStart; i < rowEnd; ++i)
      lineText += buffer.getCharAt(i);

    // Advance cursor position lookup safely
    pos = lineText.find(oldStr, pos + newStr.length());
  }

  statusMsg = "Replaced " + to_string(replaceCount) + " occurrence(s).";
}

string SearchEngine::getLastSearchPattern() { return lastSearchPattern; }