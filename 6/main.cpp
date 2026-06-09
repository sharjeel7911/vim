#include "editor.h"
#include "terminal.h"

int main(int argc, char *argv[]) {
  std::string targetFile = "";
  if (argc > 1) {
    targetFile = argv[1];
  }

  TextEditor editor;
  if (!targetFile.empty()) {
    // 1. Check if file exists, if yes, load it.
    if (editor.getFileManager().ifFileExists(targetFile)) {
      editor.getFileManager().loadFile(targetFile, editor.getBuffer());
    } else {
      // 2. If it doesn't exist, register the name as a blank new file context
      editor.getFileManager().saveFile(
          targetFile,
          editor.getBuffer()); // Creates empty file
      editor.getFileManager().loadFile(targetFile, editor.getBuffer());
    }

    // 3. CRITICAL: Force the editor status tracker to sync up with file

    editor.setupFileContext();
    editor.updateStatus(22); // Refresh cursor maps
  }

  Terminal term;

  // CHANGED: Use editor's running state instead of infinite loop
  while (editor.getifEditorRunning()) {
    // 1. Update status
    int visibleTextHeight = term.getScreenRows() - 2;
    // 2. Pass this height into updateStatus
    editor.updateStatus(visibleTextHeight);

    // size_t visibleWindowSize =
    //     term.getScreenRows() - 2; // dynamically fetch window constraint
    // editor.scrollWindow(visibleWindowSize);
    // 2. Render
    term.render(editor);

    // 3. Read key
    InputKey k = term.readKey();
    editor.handleInputFromKeyboard(k);

    // 4. Update status again after input
    editor.updateStatus(visibleTextHeight);
  }

  return 0;
}
