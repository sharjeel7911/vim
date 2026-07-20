#include "utilities.h"

#ifndef HISTORY_H
#define HISTORY_H

enum class EditType {
  INSERT_CHAR,  // a block of text was inserted   -> undo = delete it
  DELETE_CHAR,  // a block of text was deleted    -> undo = re-insert it
  ENTER_LN,     // a '\n' was inserted            -> undo = delete it
  BACKSPACE_LN, // a '\n' was deleted             -> undo = re-insert it
};

namespace UndoGroup {
  inline size_t currentId = 0;
  inline void beginGroup() { ++currentId; }
  inline size_t get() { return currentId; }
}

struct UndoAction {
  EditType type;
  size_t position;
  string text;
  size_t groupId;

  UndoAction() : type(EditType::INSERT_CHAR), position(0), text(""), groupId(0) {}
  UndoAction(EditType t, size_t pos, const string& txt) : type(t), position(pos), text(txt), groupId(0) {}
  UndoAction(EditType t, size_t pos, const string& txt, size_t gid) : type(t), position(pos), text(txt), groupId(gid) {}
};

struct UndoNode {
  UndoAction data;
  UndoNode* next;
  UndoNode(UndoAction val) : data(val), next(nullptr) {}
};

#ifndef HISTORYSTACK_H
#define HISTORYSTACK_H
class HistoryStack {
private:
  UndoNode* topNode;
public:
  HistoryStack() : topNode(nullptr) {}
  ~HistoryStack() { clear(); }
  bool isEmpty() { return topNode == nullptr; }
  void clear() { while (!isEmpty()) pop(); }
  UndoAction* peek() {
    if (isEmpty()) return nullptr;
    return &(topNode->data);
  }

  void push(UndoAction val) {
    val.groupId = UndoGroup::get();
    UndoNode* newNode = new UndoNode(val);
    newNode->next = topNode;
    topNode = newNode;
  }

  UndoAction pop() {
    if (isEmpty()) return UndoAction();
    UndoNode* temp = topNode;
    UndoAction val = topNode->data;
    topNode = topNode->next;
    delete temp;
    return val;
  }

  vector<UndoAction> popGroup() {
    vector<UndoAction> group;
    if (isEmpty()) return group;

    size_t targetId = topNode->data.groupId;
    while (!isEmpty() && topNode->data.groupId == targetId) {
      group.push_back(pop());
    }
    return group;
  }
};
#endif
#endif