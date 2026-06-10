#include "filemanager.h"

FileManager::FileManager() : currentFileName(""), modified(false) {}

bool FileManager::loadFile(const string &fileName, GapBuffer &buffer) {
  ifstream file(fileName);
  if (!file.is_open()) {
    return false;
  }

  string content;
  char ch;
  while (file.get(ch)) {
    content += ch;
  }
  file.close();
  buffer.clearBuffer();
  buffer.insertStringAtPos(0, content);

  currentFileName = fileName;
  modified = false;
  return true;
}

bool FileManager::saveFile(const string &fileName, GapBuffer &buffer) {
  ofstream file(fileName);
  if (!file.is_open()) {
    return false;
  }
  file << buffer.getWholeBufferText();
  file.close();

  currentFileName = fileName;
  modified = false;
  return true;
}

bool FileManager::saveCurrentFile(GapBuffer &buffer) {
  if (currentFileName.empty()) {
    return false;
  }
  return saveFile(currentFileName, buffer);
}

bool FileManager::hasUnsavedChanges() { return modified; }
void FileManager::markAsModified() { modified = true; }
void FileManager::markAsSaved() { modified = false; }
string FileManager::getCurrentFileName() { return currentFileName; }
bool FileManager::isFileOpen() { return !currentFileName.empty(); }
bool FileManager::ifFileExists(const string &fileName) {
  ifstream file(fileName);
  return file.good();
}

//-----------------------
