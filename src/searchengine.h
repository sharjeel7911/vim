#include "gapbuffer.h"
#include "utilities.h"

#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H
class SearchEngine {
private:
  string lastSearchPattern;
  bool caseInsensitive;
public:
  SearchEngine();

  string toLower(const string&);
  string getLastSearchPattern();
  void executeSearch(GapBuffer&, const string&, bool, size_t&, string&);

  // hooks for 'n' and 'N'
  void findNext(GapBuffer&, size_t&, string&);
  void findPrev(GapBuffer&, size_t&, string&);

  // substitution handling for :s/old/new and :s/old/new/g
  void executeSubstitution(GapBuffer&, const string&, string&);
};
#endif