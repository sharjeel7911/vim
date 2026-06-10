#include "utilities.h"

#ifndef HISTORY_H
#define HISTORY_H

// Identify the structural type of modification
enum class EditType {
  INSERT_CHAR,  // a block of text was inserted  -> undo = delete it
  DELETE_CHAR,  // a block of text was deleted   -> undo = re-insert it
  ENTER_LN,     // a '\n' was inserted            -> undo = delete it
  BACKSPACE_LN, // a '\n' was deleted             -> undo = re-insert it
};

struct UndoAction {
  EditType type;
  size_t position;
  string text;
  size_t groupId; // CHANGED: all actions from one user command share the same
                  // groupId

  UndoAction()
      : type(EditType::INSERT_CHAR), position(0), text(""), groupId(0) {}
  UndoAction(EditType t, size_t pos, const string &txt)
      : type(t), position(pos), text(txt), groupId(0) {}
  UndoAction(EditType t, size_t pos, const string &txt, size_t gid)
      : type(t), position(pos), text(txt), groupId(gid) {}
};

// CHANGED: Central group counter — call beginGroup() once per user command
// before any pushes. All UndoActions pushed after beginGroup() get the same
// groupId until the next beginGroup().
namespace UndoGroup {
inline size_t currentId = 0;
inline void beginGroup() { ++currentId; }
inline size_t get() { return currentId; }
} // namespace UndoGroup

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

  // CHANGED: push now auto-stamps the current groupId onto every action
  void push(UndoAction val) {
    val.groupId = UndoGroup::get(); // stamp group
    UndoNode *newNode = new UndoNode(val);
    newNode->next = topNode;
    topNode = newNode;
  }

  UndoAction pop() {
    if (isEmpty())
      return UndoAction();
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

  // CHANGED: popGroup() pops every action that shares the top groupId.
  // Returns them in the order they should be replayed (reversed from stack
  // order).
  vector<UndoAction> popGroup() {
    vector<UndoAction> group;
    if (isEmpty())
      return group;

    size_t targetId = topNode->data.groupId;
    while (!isEmpty() && topNode->data.groupId == targetId) {
      group.push_back(pop());
    }
    return group;
  }

  void clear() {
    while (!isEmpty())
      pop();
  }
};

#endif