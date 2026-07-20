#include "searchengine.h"

SearchEngine::SearchEngine() : lastSearchPattern("") {}

void SearchEngine::setLastPattern(const string &pattern) {
  lastSearchPattern = pattern;
}

string SearchEngine::getLastPattern() { return lastSearchPattern; }

size_t SearchEngine::searchForward(string &pattern, size_t startPos,
                                   GapBuffer &buffer) {
  if (pattern.empty())
    return string::npos;

  string text = buffer.getWholeBufferText();
  if (startPos >= text.length())
    startPos = 0; // Wrap search around to the top

  size_t found = text.find(pattern, startPos);
  if (found == string::npos && startPos > 0) {
    found = text.find(pattern, 0);
  }
  return found;
}

size_t SearchEngine::searchBackward(string &pattern, size_t startPos,
                                    GapBuffer &buffer) {
  if (pattern.empty())
    return string::npos;

  string text = buffer.getWholeBufferText();
  if (startPos == 0)
    startPos = text.length();

  // Look backward starting slightly left of the cursor position
  size_t searchStart =
      (startPos == text.length()) ? text.length() : startPos - 1;
  size_t found = text.rfind(pattern, searchStart);

  if (found == string::npos && startPos < text.length()) {
    found = text.rfind(pattern, text.length());
  }
  return found;
}

void SearchEngine::findNext(size_t &cursorPos, GapBuffer &buffer) {
  if (lastSearchPattern.empty())
    return;

  // Start searching 1 character forward to avoid getting stuck on current match
  size_t target = searchForward(lastSearchPattern, cursorPos + 1, buffer);
  if (target != string::npos) {
    cursorPos = target;
  }
}

void SearchEngine::findPrev(size_t &cursorPos, GapBuffer &buffer) {
  if (lastSearchPattern.empty())
    return;

  size_t target = searchBackward(lastSearchPattern, cursorPos, buffer);
  if (target != string::npos) {
    cursorPos = target;
  }
}

// bool SearchEngine::substituteOnCurrentLine(const string &subCommand,
//                                            size_t cursorPos,
//                                            GapBuffer &buffer) {
//   // Expected format examples: "s/old/new" or "s/old/new/g"
//   if (subCommand.length() < 4 || subCommand[0] != 's' || subCommand[1] !=
//   '/') {
//     return false;
//   }

//   // Parse the components out via the '/' delimiter bounds
//   vector<string> tokens;
//   string currentToken = "";

//   for (size_t i = 2; i < subCommand.length(); ++i) {
//     if (subCommand[i] == '/') {
//       tokens.push_back(currentToken);
//       currentToken = "";
//     } else {
//       currentToken += subCommand[i];
//     }
//   }
//   // Catch the trailing string or 'g' modifier flag
//   tokens.push_back(currentToken);

//   if (tokens.size() < 2)
//     return false; // Validation safeguard

//   string oldStr = tokens[0];
//   string newStr = tokens[1];
//   bool global = (tokens.size() > 2 && tokens[2] == "g");

//   if (oldStr.empty())
//     return false;

//   // 1. Isolate the absolute text limits of the current row bounds
//   string fullText = buffer.getWholeBufferText();

//   size_t lineStart = fullText.rfind('\n', (cursorPos > 0) ? cursorPos - 1 :
//   0); lineStart = (lineStart == string::npos) ? 0 : lineStart + 1;

//   size_t lineEnd = fullText.find('\n', cursorPos);
//   lineEnd = (lineEnd == string::npos) ? fullText.length() : lineEnd;

//   string lineContent = fullText.substr(lineStart, lineEnd - lineStart);

//   // 2. Perform replacement operations on that line segment
//   size_t replacePos = lineContent.find(oldStr);
//   if (replacePos == string::npos)
//     return false; // Pattern not found on this line

//   if (global) {
//     while (replacePos != string::npos) {
//       lineContent.replace(replacePos, oldStr.length(), newStr);
//       replacePos = lineContent.find(oldStr, replacePos + newStr.length());
//     }
//   } else {
//     lineContent.replace(replacePos, oldStr.length(), newStr);
//   }

//   // 3. Reconstruct the buffer contents safely
//   string upgradedText =
//       fullText.substr(0, lineStart) + lineContent + fullText.substr(lineEnd);
//   buffer.clearBuffer();
//   buffer.insertStringAtPos(0, upgradedText);

//   return true;
// }