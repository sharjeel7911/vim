#include "gapbuffer.h"
#include <vector>

#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

class SearchEngine {
private:
  string lastSearchPattern;

public:
  SearchEngine();

  // Core navigation searches (Returns absolute buffer index of match, or
  // string::npos if not found)
  size_t searchForward(string &, size_t, GapBuffer &);
  size_t searchBackward(string &, size_t, GapBuffer &);

  // Repeat functions for 'n' and 'N' keys
  void findNext(size_t &, GapBuffer &);
  void findPrev(size_t &, GapBuffer &);

  // Regex/Literal substitutions (:s/old/new and :s/old/new/g)
  bool substituteOnCurrentLine(const string &, size_t, GapBuffer &);

  // Setters/Getters for the pattern memory state
  void setLastPattern(const string &);
  string getLastPattern();
};

#endif