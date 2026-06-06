#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "gapbuffer.h"
#include "utilities.h"

class SearchEngine {
private:
  string lastSearchPattern;
  bool caseInsensitive;

  // Helper to convert strings to lowercase for case-insensitive searching
  string toLower(const string &str);

public:
  SearchEngine();

  // Core execution method requested
  void executeSearch(GapBuffer &buffer, const string &pattern, bool forward,
                     size_t &statusMsgRow, string &statusMsg);

  // Hooks for 'n' and 'N'
  void findNext(GapBuffer &buffer, size_t &statusMsgRow, string &statusMsg);
  void findPrev(GapBuffer &buffer, size_t &statusMsgRow, string &statusMsg);

  // Substitution handling for :s/old/new and :s/old/new/g
  void executeSubstitution(GapBuffer &buffer, const string &commandStr,
                           string &statusMsg);

  string getLastSearchPattern();
};

#endif