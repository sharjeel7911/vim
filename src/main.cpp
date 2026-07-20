#include "editor.h"
#include "terminal.h"

int main(int argc, char* argv[]) {
  string targetFile = "";
  if (argc > 1) {
    targetFile = argv[1];
  }

  TextEditor editor;
  if (!targetFile.empty()) {
    // check if file exists, if yes, load it.
    if (editor.getFileManager().ifFileExists(targetFile)) {
      editor.getFileManager().loadFile(targetFile, editor.getBuffer());
    }
    else {
      // if it doesn't exist, register the name as a blank new file context
      editor.getFileManager().saveFile(targetFile, editor.getBuffer()); // creates empty file
      editor.getFileManager().loadFile(targetFile, editor.getBuffer());
    }
    editor.setupFileContext();
    editor.updateStatus(22);
  }

  Terminal term;
  while (editor.getifEditorRunning()) {
    int visibleTextHeight = term.getScreenRows() - 2; // update status
    editor.updateStatus(visibleTextHeight); // pass this height into updateStatus
    term.render(editor); // render
    InputKey k = term.readKey(); // read key from keyboard
    editor.handleInputFromKeyboard(k); // interpret the key
    editor.updateStatus(visibleTextHeight); // update status again after input
  }
  return 0;
}