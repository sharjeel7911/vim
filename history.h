#include "utilities.h"

#ifndef HISTORY_H
#define HISTORY_H

// Identify the structural type of modification
enum class EditType {
  INSERT_CHAR,
  DELETE_CHAR,
  ENTER_LN,
  BACKSPACE_LN,

  // New structural types for row operations
  // DELETE_BLOCK, // Tracks text blocks removed via dd, cc, or row deletions
  // INSERT_BLOCK, // Tracks text blocks added via paste (p / P)
  // JOIN_BLOCK    // Tracks row joins via J (removes a newline at a specific
  // spot)
};

struct UndoAction {
  EditType type;   // Action descriptor
  size_t position; // Index position where the modification took place
  string text;     // Captured text context

  UndoAction() : type(EditType::INSERT_CHAR), position(0), text("") {}
  UndoAction(EditType t, size_t pos, const string &txt)
      : type(t), position(pos), text(txt) {}
};

class UndoNode {
public:
  UndoAction data;
  UndoNode *next;
  UndoNode(UndoAction val) : data(val), next(nullptr) {}
};

class HistoryStack {
private:
  UndoNode *topNode;

public:
  HistoryStack() : topNode(nullptr) {}
  ~HistoryStack() { clear(); }
  bool isEmpty() { return topNode == nullptr; }

  void push(UndoAction val) {
    UndoNode *newNode = new UndoNode(val);
    newNode->next = topNode;
    topNode = newNode;
  }

  UndoAction pop() {
    if (isEmpty()) {
      return UndoAction();
    }
    UndoNode *temp = topNode;
    UndoAction val = topNode->data;
    topNode = topNode->next;
    delete temp;
    return val;
  }

  UndoAction *peek() {
    if (isEmpty())
      return nullptr;
    return &(topNode->data);
  }

  void clear() {
    while (!isEmpty()) {
      pop();
    }
  }
};

#endif