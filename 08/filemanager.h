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

  bool loadFile(const string&, GapBuffer&);
  bool saveFile(const string&, GapBuffer&);
  bool saveCurrentFile(GapBuffer&);

  bool hasUnsavedChanges();
  void markAsModified();
  void markAsSaved();

  string getCurrentFileName();
  bool isFileOpen();

  static bool ifFileExists(const string&);
};
#endif