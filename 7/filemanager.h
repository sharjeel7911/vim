#include "gapbuffer.h"
#include "utilities.h"
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

class FileManager {
private:
  string currentFileName;
  bool modified;

public:
  FileManager();

  // Core file operations
  bool loadFile(const string &, GapBuffer &);
  bool saveFile(const string &, GapBuffer &);
  bool saveCurrentFile(GapBuffer &);

  // Modified tracking
  bool hasUnsavedChanges();
  void markAsModified();
  void markAsSaved();

  // Getters
  string getCurrentFileName();
  bool isFileOpen();

  static bool ifFileExists(const string &);
};

#endif