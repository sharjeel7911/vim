//AHMED's code
#include <iostream>
#include<string>
#include <vector>
#include <cstdlib>
#include<fstream>
#include<conio.h>
#include<windows.h>
using namespace std;

void SetColor(int textColor) {
    cout << "\033[" << textColor << "m";
}

void ResetColor() {
    cout << "\033[0m";
}

bool isWordCharacter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

bool isPunctuation(char c) {
    return (c == '.' || c == ',' || c == '!' || c == '?' || c == ';' || c == ':' ||
        c == '-' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' ||
        c == '\'' || c == '"' || c == '/');
}

class DoubleLinkList { // double link list representing every line
    class Node {
    public:
        char val;
        Node* nxt;
        Node* prv;
        Node(char value, Node* next = nullptr, Node* prev = nullptr) : val(value), nxt(next), prv(prev) {}
    }; //Basic node class for doublelinklist

    Node* first;
    Node* last;
    Node* cursor;

public:
    DoubleLinkList() {
        cursor = new Node('|');
        first = last = cursor;// we will start every line with cursor at start
    }

    class Iterator {
        Node*& curr;

    public:
        Iterator(Node*& current) : curr(current) {}

        Iterator& operator++(int v) {//in simple terms moving right
            if (curr && curr->nxt) {//we are swapping values with the right node and moving current to the cursor
                char n = curr->nxt->val;
                char c = curr->val;
                curr->val = n;
                curr->nxt->val = c;
                curr = curr->nxt;
            }
            return *this;
        }
        Iterator& operator--(int v) {//this is moving left
            if (curr && curr->prv) {//again swapping from left and moving current pointer to the cursor
                char p = curr->prv->val;
                char c = curr->val;
                curr->val = p;
                curr->prv->val = c;
                curr = curr->prv;
            }
            return *this;
        }

        Node* getCursorNext() {//get next used in conditions of while loops when ++ 
            return curr->nxt;
        }
        Node* getCursorPrev() {// used in conditions of while loops when --
            return curr->prv;
        }
        bool isCursorAtEnd() {
            return curr->nxt == nullptr;
        }
        bool isCursorAtStart() {
            return curr->prv == nullptr;
        }
    };

    void insertAtCursor(char val) {//inserting at cursor by adding a new node between cursor and cursor->prev
        Node* newNode = new Node(val);
        newNode->nxt = cursor;
        newNode->prv = cursor->prv;
        if (cursor->prv) {
            cursor->prv->nxt = newNode;
        } else {
            first = newNode;
        }
        cursor->prv = newNode;
        if (cursor == last) {
            last = cursor;
        }
    }

    void deleteAtCursor() {//delete node between cursor and cursor->prev->prev
        if (!cursor || cursor == first) return;
        Node* toDelete = cursor->prv;
        if (toDelete->prv) {
            toDelete->prv->nxt = cursor;
        } else {
            first = cursor;
        }
        cursor->prv = toDelete->prv;
        delete toDelete;
    }
    void deleteAfterCursor() {//delete node between cursor and cursor->next->next
        if (!cursor || cursor == last) return;
        Node* toDelete = cursor->nxt;
        if (toDelete->nxt) {
            toDelete->nxt->prv = cursor;
        } else {
            last = cursor;
        }
        cursor->nxt = toDelete->nxt;
        delete toDelete;
    }
    void clearLinkList() {
        Node* temp = first;
        while (temp) {
            Node* toDelete = temp;
            temp = temp->nxt;
            delete toDelete;
        }
        first = last = cursor = new Node('|');
    }


    void hideCursor() {// this is to hide when the line is not current line
        Iterator c = getCursor();
        while (c.getCursorNext()) {//moves cursor pointer to the last and makes it blank
            c++;
        }
        cursor->val = ' ';
    }

    void showCursor() {//changes cursor from blank to '|'
        cursor->val = '|';
    }

    void print() {// printing particular line
        Node* temp = first;
        while (temp) {
            if (temp->val != '|') {
                SetColor(97);
                cout << temp->val;
                ResetColor();
            } else {
                SetColor(5);
                cout << temp->val;
                ResetColor();
            }
            temp = temp->nxt;
        }
        cout << endl;
    }

    vector<int> printWithFind(string find) {
        Node* temp = first;
        string content;
        vector<int> foundPositions;


        while (temp) {
            content += temp->val;
            temp = temp->nxt;
        }

        int pos = 0;
        int findPos = content.find(find, pos);
        temp = first;
        pos = 0;

        while (temp) {
            if (pos == findPos) {
                foundPositions.push_back(pos);
                SetColor(43);
                for (int i = 0; i < find.size(); ++i) {
                    cout << temp->val;
                    temp = temp->nxt;
                    ++pos;
                }
                ResetColor();
                findPos = content.find(find, pos);
            } else {

                if (temp->val == '|') {
                    SetColor(5);
                } else {
                    SetColor(97);
                }
                cout << temp->val;
                ResetColor();
                temp = temp->nxt;
                ++pos;
            }
        }

        cout << endl;
        return foundPositions;
    }



    Iterator getCursor() { //getting cursor of the line
        return Iterator(cursor);
    }

    int getCursorCol() {
        int counter = 0;
        Node* temp = first;
        while (temp != cursor) {
            if (temp == nullptr) {
                return -1;
            }
            counter++;
            temp = temp->nxt;
        }
        return counter;
    }

    int getCursorColFromEnd() {
        int counter = 0;
        Node* temp = last;

        while (temp != cursor) {
            if (temp == nullptr) {
                return -1;
            }
            counter++;
            temp = temp->prv;
        }
        return counter;
    }

    int getWords() {
        int counter = 0;
        if (first == nullptr) {
            return 0;
        }
        Node* temp = first;

        if (temp == nullptr) {
            return 0;
        }

        while (temp != nullptr) {
            if (isWordCharacter(temp->val)) {
                counter++;
                while (temp != nullptr && isWordCharacter(temp->val)) {
                    temp = temp->nxt;
                }
            }
            while (temp != nullptr && !isWordCharacter(temp->val)) {
                temp = temp->nxt;
            }
        }

        return counter;
    }
    bool isEmpty() const {
        return first == cursor && last == cursor;
    }

};

class TextEditor {// text editor that merges double link lists in vector
    vector<DoubleLinkList> editor;
    int currentLine;
    bool insertMode;//keep track of modes

    string copyBuffer;
    string filename = "";

    string findText = "";
    vector<pair<int, int>> finds = {};
    int findIndex;


public:
    bool saved = false;
    TextEditor() : currentLine(0), insertMode(false) {//starts with 1 line and normal mode
        editor.push_back(DoubleLinkList());
    }

    void toggleMode() {//toggling between insert and normal
        insertMode = !insertMode;
    }
    bool isInsert() {//getter of insertMode
        return insertMode;
    }

    void writeChar(char c) {//simply calls the insert func of current line
        if (currentLine < editor.size()) {
            editor[currentLine].insertAtCursor(c);
        }
    }

    void delChar() {//simply calls the del func of current line
        if (insertMode && currentLine < editor.size()) {
            editor[currentLine].deleteAtCursor();
        }
    }

    //hides cursor of current line and makes new double link list below current
    void enterPressed() {
        if (insertMode) {
            string cutLine;
            auto cursor = editor[currentLine].getCursor();
            while (cursor.getCursorNext()) {
                cursor++;
                cutLine += cursor.getCursorPrev()->val;
                editor[currentLine].deleteAtCursor();
            }

            editor[currentLine].hideCursor();

            editor.insert(editor.begin() + currentLine + 1, DoubleLinkList());
            currentLine++;
            for (char c : cutLine) {
                editor[currentLine].insertAtCursor(c);
            }
            moveToStartOfLine();
        }
    }

    void moveUp() {//moves while hiding and showing
        if (currentLine > 0) {
            editor[currentLine].hideCursor();
            currentLine--;
            editor[currentLine].showCursor();
        }
    }

    void moveDown() {//moves while hiding and showing
        if (currentLine < editor.size() - 1) {
            editor[currentLine].hideCursor();
            currentLine++;
            editor[currentLine].showCursor();
        }
    }
    void moveUpPerfect() {//moves while hiding and showing
        if (currentLine > 0) {
            int cursorPos = editor[currentLine].getCursorCol();
            editor[currentLine].hideCursor();
            currentLine--;
            int newcursorPos = editor[currentLine].getCursorCol();
            if (newcursorPos > cursorPos) {
                auto cursor = editor[currentLine].getCursor();
                while (cursorPos != newcursorPos) {
                    cursor--;
                    newcursorPos--;
                }
            }
            editor[currentLine].showCursor();
        }

    }

    void moveDownPerfect() {//moves while hiding and showing
        if (currentLine < editor.size() - 1) {
            int cursorPos = editor[currentLine].getCursorCol();
            editor[currentLine].hideCursor();
            currentLine++;
            int newcursorPos = editor[currentLine].getCursorCol();
            if (newcursorPos > cursorPos) {
                auto cursor = editor[currentLine].getCursor();
                while (cursorPos != newcursorPos) {
                    cursor--;
                    newcursorPos--;
                }
            }
            editor[currentLine].showCursor();
        }
    }


    void moveLeft() {//moves while hiding and showing
        if (currentLine < editor.size()) {
            auto cursor = editor[currentLine].getCursor();
            cursor--;
        }
    }

    void moveRight() {//moves while hiding and showing
        if (currentLine < editor.size()) {
            auto cursor = editor[currentLine].getCursor();
            cursor++;
        }
    }

    void deleteCurrentLine() {
        if (editor.size() > 1) {
            editor.erase(editor.begin() + currentLine);
            if (currentLine == editor.size()) {
                currentLine--;
            }
            editor[currentLine].showCursor();
        } else if (editor.size() == 1) {
            editor[currentLine].clearLinkList();
        }

    } // for 'dd' command
    void deleteToEndOfLine() {
        auto cursor = editor[currentLine].getCursor();
        while (cursor.getCursorNext()) {
            cursor++;
            editor[currentLine].deleteAtCursor();
        }
    } // for 'D' command
    void deleteCharacterAtCursor() {
        auto cursor = editor[currentLine].getCursor();
        if (cursor.getCursorNext()) {
            editor[currentLine].deleteAfterCursor();
        } else {
            editor[currentLine].deleteAtCursor();
        }

    } // for 'x' command
    void backspace() {
        if (editor[currentLine].getCursorCol() == 0) {
            //for moving current line up when cursor is at start
            if (currentLine > 0) {
                string cutLine;
                auto cursor = editor[currentLine].getCursor();
                while (cursor.getCursorNext()) {
                    cutLine += cursor.getCursorNext()->val;
                    cursor++;
                }
                editor[currentLine].clearLinkList();
                auto upperCursor = editor[currentLine - 1].getCursor();
                while (upperCursor.getCursorNext()) {
                    upperCursor++;
                }
                for (char c : cutLine) {
                    editor[currentLine - 1].insertAtCursor(c);
                    upperCursor++;
                }
                for (char c : cutLine) {
                    upperCursor--;
                }
                editor.erase(editor.begin() + currentLine);
                currentLine--;
                editor[currentLine].showCursor();
            }
        } else {
            editor[currentLine].deleteAtCursor();
        }

    }


    void yankLine() {
        copyBuffer.clear();
        auto cursor = editor[currentLine].getCursor();
        auto temp = cursor.getCursorPrev();

        while (temp && temp->prv) {
            temp = temp->prv;
        }

        while (temp) {
            if (temp->val != '|') {
                copyBuffer += temp->val;
            }
            temp = temp->nxt;
        }
    }
    void yankLine(int count) {
        copyBuffer.clear();


        int lineIndex = currentLine;


        while (lineIndex < editor.size() && count > 0) {
            auto cursor = editor[lineIndex].getCursor();
            auto temp = cursor.getCursorPrev();


            while (temp && temp->prv) {
                temp = temp->prv;
            }


            while (temp) {
                if (temp->val != '|') {
                    copyBuffer += temp->val;
                }
                temp = temp->nxt;
            }

            lineIndex++;
            count--;
        }
    }
    void pasteAfter() {
        if (copyBuffer.empty() == false) {
            DoubleLinkList newLine;
            for (int i = 0; i < copyBuffer.size(); i++) {
                newLine.insertAtCursor(copyBuffer[i]);
            }
            editor.insert(editor.begin() + currentLine + 1, newLine);
            editor[currentLine + 1].hideCursor();
        }
    }
    void pasteBefore() {
        if (copyBuffer.empty() == false) {
            DoubleLinkList newLine;
            for (int i = 0; i < copyBuffer.size(); i++) {
                newLine.insertAtCursor(copyBuffer[i]);
            }
            editor.insert(editor.begin() + currentLine, newLine);
            editor[currentLine].hideCursor();
            currentLine++;
        }
    }

    void moveToStartOfLine() {
        auto cursor = editor[currentLine].getCursor();
        while (cursor.getCursorPrev()) {
            cursor--;
        }
    }

    void moveToEndOfLine() {
        auto cursor = editor[currentLine].getCursor();
        while (cursor.getCursorNext()) {
            cursor++;
        }
    }


    void moveToNextWord() {
        auto cursor = editor[currentLine].getCursor();
        while (cursor.getCursorNext() && !isWordCharacter(cursor.getCursorNext()->val)) {
            cursor++;
        }
        while (cursor.getCursorNext() && isWordCharacter(cursor.getCursorNext()->val)) {
            cursor++;
        }
        while (cursor.getCursorNext() && !isWordCharacter(cursor.getCursorNext()->val)) {
            cursor++;
        }
        if (cursor.isCursorAtEnd()) {
            if (currentLine != editor.size() - 1) {
                moveDown();
                moveToStartOfLine();
                auto newcursor = editor[currentLine].getCursor();
                while (newcursor.getCursorNext() && !isWordCharacter(newcursor.getCursorNext()->val)) {
                    newcursor++;
                }
            }
        }
    }

    void moveToPreviousWord() {

        auto cursor = editor[currentLine].getCursor();
        if (cursor.isCursorAtStart()) {
            if (currentLine != 0) {
                moveUp();
                auto newcursor = editor[currentLine].getCursor();
                while (newcursor.getCursorPrev() && !isWordCharacter(newcursor.getCursorPrev()->val)) {
                    newcursor--;
                }
                while (newcursor.getCursorPrev() && isWordCharacter(newcursor.getCursorPrev()->val)) {
                    newcursor--;
                }
            }
        }

        while (cursor.getCursorPrev() && !isWordCharacter(cursor.getCursorPrev()->val)) {
            cursor--;
        }
        while (cursor.getCursorPrev() && isWordCharacter(cursor.getCursorPrev()->val)) {
            cursor--;
        }
    }
    void moveToWordEnd() {
        auto cursor = editor[currentLine].getCursor();
        if (cursor.isCursorAtEnd()) {
            if (currentLine != editor.size() - 1) {
                moveDown();
                moveToStartOfLine();
                auto newcursor = editor[currentLine].getCursor();
                while (newcursor.getCursorNext() && !isWordCharacter(newcursor.getCursorNext()->val)) {
                    newcursor++;
                }
                while (newcursor.getCursorNext() && isWordCharacter(newcursor.getCursorNext()->val)) {
                    newcursor++;
                }
            }
        }
        while (cursor.getCursorNext() && !isWordCharacter(cursor.getCursorNext()->val)) {
            cursor++;
        }
        while (cursor.getCursorNext() && isWordCharacter(cursor.getCursorNext()->val)) {
            cursor++;
        }

    }
    void deleteLine(int lineNumber) {
        if (lineNumber - 1 >= 0 && lineNumber - 1 < editor.size()) {
            editor[currentLine].hideCursor();
            if (editor.size() > 1) {
                editor.erase(editor.begin() + (lineNumber - 1));
            } else {
                editor[currentLine].clearLinkList();
            }
            currentLine = editor.size() - 1;
            if (!editor.empty()) {
                editor[currentLine].showCursor();
            }
        }
    }
    void joinLineWithNext() {
        if (currentLine < editor.size() - 1) {
            auto cursor = editor[currentLine + 1].getCursor();
            auto currentcursor = editor[currentLine].getCursor();
            while (currentcursor.getCursorNext()) {
                currentcursor++;
            }
            while (cursor.getCursorPrev()) {
                cursor--;
            }
            int counter = 0;
            while (cursor.getCursorNext()) {
                writeChar(cursor.getCursorNext()->val);
                cursor++;
                counter++;
            }
            for (int i = 0; i < counter; i++) {
                currentcursor--;
            }
            editor.erase(editor.begin() + (currentLine + 1));
        }
    }
    void indentLine() {
        moveToStartOfLine();
        for (int i = 0; i < 4; i++) {
            writeChar(' ');
        }
        auto cursor = editor[currentLine].getCursor();
        while (cursor.getCursorNext() && cursor.getCursorNext()->val == ' ') {
            cursor++;
        }
    }
    void unindentLine() {
        auto cursor = editor[currentLine].getCursor();
        moveToStartOfLine();
        int counter = 0;
        while (cursor.getCursorNext() && cursor.getCursorNext()->val == ' ' && counter < 4) {
            editor[currentLine].deleteAfterCursor();
            counter++;
        }
        while (cursor.getCursorNext() && cursor.getCursorNext()->val == ' ') {
            cursor++;
        }
    }

    void executeWithCount(int count, const std::string& cmd) {
        if (cmd == "dd") {
            for (int i = 0; i < count; ++i) {
                deleteCurrentLine();
                if (editor.size() == 1 && editor[currentLine].isEmpty()) {
                    break;
                }
                if (editor.size() == 1 && !editor[currentLine].isEmpty()) {
                    editor[currentLine].clearLinkList();
                    break;
                }
            }
        } else if (cmd == "yy") {
            copyBuffer.clear();
            yankLine(count);
        } else if (cmd == "j") {
            for (int i = 0; i < count; ++i) {
                moveDown();
            }
        } else if (cmd == "k") {
            for (int i = 0; i < count; ++i) {
                moveUp();
            }
        }

    }

    /*void saveToFile(const string& filename, const string& data) {

        ofstream file(filename, ios::out);
        if (!file) {
            cerr << "Error: Could not create or open the file " << filename << "\n";
            return;
        }
        file << data;
        file.close();

        cout << "Data successfully saved to " << filename << "\n";
    }*/

    void saveToFile(const std::string& fname = "") {
        std::string filepath;

        if (fname.empty()) {
            if (!filename.empty()) {
                filepath = filename;
            } else {
                cout << "Error: No file specified!" << endl;
            }
        } else {
            filepath = fname;
        }

        ofstream file(filepath, ios::out);
        if (!file) {
            std::cerr << "Error: Could not open file for writing.\n";
            return;
        }

        for (int i = 0; i < editor.size(); i++) {
            currentLine = i;
            auto cursor = editor[i].getCursor();
            moveToStartOfLine();
            while (cursor.getCursorNext()) {
                cursor++;
                if (cursor.getCursorPrev()->val != '|') {
                    file << cursor.getCursorPrev()->val;
                }
            }
            file << '\n';
        }
        file.close();
        std::cout << "File saved to Source Files as " << filename << "\n";
        saved = true;
    }


    void openFile(const std::string& fname) {
        filename = fname;
        ifstream file(filename);
        if (!file) {
            cerr << "Error: Could not open file for reading.\n";
            return;
        }
        editor.clear();
        currentLine = 0;
        std::string line;
        while (std::getline(file, line)) {
            DoubleLinkList newLine;
            for (char c : line) {
                newLine.insertAtCursor(c);
            }
            newLine.hideCursor();
            editor.push_back(newLine);
        }
        editor.back().showCursor();
        currentLine = editor.size() - 1;
        file.close();
        if (editor.empty()) {
            editor.push_back(DoubleLinkList());
        }
        display('e', ":e ");
        cout << "File loaded: " << filename << "\n";
        saved = true;
    }
    int getWords() {
        return editor[currentLine].getWords();
    }

    void findAndDisplay(char searchEndingChar) {
        finds.clear();
        string cmd = searchEndingChar == '/' ? ":s/" : "/ ";
        findText = "";
        display('/', cmd);
        char x = _getch();
        while (x != searchEndingChar) {
            if (x == 8) {
                if (!findText.empty()) {
                    findText.pop_back();
                    cmd.pop_back();
                }
            } else {
                findText += x;
                cmd += x;

            }
            display(x, cmd, "");
            x = _getch();
        }
    }
    void replaceFindText() {
        string cmd = ":s/" + findText + "/";
        string replaceText = "";
        display('/', cmd);
        char x = _getch();
        while (x != 13 && x != '/') {
            if (x == 8) {
                if (!replaceText.empty()) {
                    replaceText.pop_back();
                    cmd.pop_back();
                }
            } else {
                replaceText += x;
                cmd += x;

            }
            display(x, cmd, "");
            x = _getch();
        }
        if (!replaceText.empty() && !finds.empty()) {
            if (x == 13) {
                moveToPosition(finds[0].first, finds[0].second);
                for (int i = 0; i < findText.size(); i++) {
                    editor[currentLine].deleteAfterCursor();
                }
                for (int i = 0; i < replaceText.size(); i++) {
                    editor[currentLine].insertAtCursor(replaceText[i]);
                }
                finds.erase(finds.begin());
            }
            if (x == '/') {
                for (int f = 0; f < finds.size(); f++) {
                    moveToPosition(finds[f].first, finds[f].second);
                    for (int i = 0; i < findText.size(); i++) {
                        editor[currentLine].deleteAfterCursor();
                    }
                    for (int i = 0; i < replaceText.size(); i++) {
                        editor[currentLine].insertAtCursor(replaceText[i]);
                    }
                }
                finds.clear();
            }
        }
    }

    void moveToPosition(int targetLine, int targetColumn) {
        while (currentLine != targetLine) {
            if (targetLine < currentLine) {
                moveUp();
            } else {
                moveDown();
            }
        }
        while (editor[currentLine].getCursorCol() != targetColumn) {
            if (targetColumn < editor[currentLine].getCursorCol()) {
                moveLeft();
            } else {
                moveRight();
            }
        }
    }


    void moveToNextSearch() {

        if (findIndex < finds.size() - 1 && findIndex >= 0) {
            findIndex++;
        } else {
            findIndex = 0;
        }
        int line = finds[findIndex].first;
        int column = finds[findIndex].second;
        while (line != currentLine) {
            if (line < currentLine) {
                moveUp();
            } else {
                moveDown();
            }
        }
        while (column != editor[currentLine].getCursorCol()) {
            if (column < editor[currentLine].getCursorCol()) {
                moveLeft();
            } else {
                moveRight();
            }
        }

    }

    void moveToPrevSearch() {

        if (findIndex < finds.size() && findIndex > 0) {
            findIndex--;
        } else {
            findIndex = 0;
        }
        int line = finds[findIndex].first;
        int column = finds[findIndex].second;
        while (line != currentLine) {
            if (line < currentLine) {
                moveUp();
            } else {
                moveDown();
            }
        }
        while (column != editor[currentLine].getCursorCol()) {
            if (column < editor[currentLine].getCursorCol()) {
                moveLeft();
            } else {
                moveRight();
            }
        }
    }

    void display(char key, string speacial = "", string warning = "") {
        //my display func that refreshes in while loop
        if (!findText.empty()) {
            finds.clear();
        }
        system("cls");
        SetColor(90);
        SetColor(3);
        cout << (insertMode ? "Insert" : "Normal") << " | " << (filename.empty() ? "No file!" : filename);
        ResetColor();
        saved ? SetColor(90) : SetColor(34);
        SetColor(1);
        cout << (saved ? " [-]" : " [+]");
        ResetColor();
        SetColor(90);
        SetColor(3);
        cout << " | Line: " << currentLine << ", Col: " << editor[currentLine].getCursorCol() << "\nTotal Lines: " << editor.size() << endl << "Words in line: " << getWords() << endl << endl;
        ResetColor();
        cout << "_______NotePad_______\n\n";
        for (int i = 0; i < editor.size(); ++i) {
            if (findText.empty()) {
                cout << i + 1 << ")";
                editor[i].print();
            } else {
                cout << i + 1 << ")";
                vector<int> cols = editor[i].printWithFind(findText);
                for (int c : cols) {
                    finds.push_back(make_pair(i, c));
                }
            }
        }

        cout << "_____________________\n";

        string k = " ";
        switch (key) {//writing key log in better way for speacial char
        case 38:
            k = "up";
            break;
        case 40:
            k = "down";
            break;
        case 37:
            k = "left";
            break;
        case 39:
            k = "right";
            break;
        case 13:
            k = "enter";
            break;
        case 8:
            k = "backspace";
            break;
        case 27:
            k = "esc";
            break;
        default:
            k = key;
        }
        SetColor(90);
        SetColor(3);
        cout << "\nKey pressed: " << k << endl;
        speacial == "" ? cout << "" : cout << "Command: " << speacial << endl;
        ResetColor();
        if (!warning.empty()) {
            SetColor(4);
            SetColor(31);
            cout << "Warning" << endl;
            ResetColor();
            SetColor(91);
            cout << "" << warning << endl;
            ResetColor();
        }
    }
};

int getKeys() {
    int ch = _getch();

    if (ch == 0 || ch == 224) {  //for arrow keys
        ch = _getch();
        switch (ch) {
        case 72: return 38; //up 
        case 80: return 40; //down 
        case 75: return 37; //left 
        case 77: return 39; //right 
        default: return ch + 256;
        }
    }
    //other keys
    switch (ch) {
    case 13: return 13;   // Enter
    case 8:  return 8;    // Backspace
    case 27: return 27;   // ESC
    case 'i': return 'i'; // Insert mode toggle
    case 'q': return 'q'; // Quit
    case 'd': return 'd'; // del current line (dd)
    case 'D': return 'D'; // del to end of line
    case 'x': return 'x'; // del character at cursor
    case 'y': return 'y'; // copy current line (yy)
    case 'p': return 'p'; // paste after current line
    case 'P': return 'P'; // paste before current line
    case '0': return '0'; //  start of line
    case '$': return '$'; //  end of line
    case 'w': return 'w'; //  start of next word
    case 'b': return 'b'; //  start of previous word
    case 'e': return 'e'; //  end of word
    case 'J': return 'J'; // join
    case '>': return '>'; // indent
    case '<': return '<'; // remove indent
    case ':': return ':'; //speacial kaam hone wala
    case '/': return '/'; // search
    case 'n': return 'n';
    case 'N': return 'N';
    default: return ch;   // all other
    }
}

int main() {
    TextEditor editor; // initialize editor
    int command = ' ';
    char firstPressed = ' '; // for 'dd', '<<", ">>"
    int commandCount = 1;
    bool missDisplay = false;
    bool searching = false;

    while (true) {
        if (!missDisplay) {
            editor.display(command);
        }// Display screen
        missDisplay = false;
        command = getKeys(); // Get key input

        if (editor.isInsert()) { // Insert mode commands
            editor.saved = false;
            switch (command) {
            case 27: // ESC to switch to normal mode
                editor.toggleMode();
                break;
            case 38: // Up
                editor.moveUpPerfect();
                break;
            case 40: // Down
                editor.moveDownPerfect();
                break;
            case 37: // Left
                editor.moveLeft();
                break;
            case 39: // Right
                editor.moveRight();
                break;
            case 13: // Enter
                editor.enterPressed();
                break;
            case 8: // Backspace
                editor.backspace();
                break;
            default: // Insert character
                editor.writeChar(static_cast<char>(command));
                break;
            }
        } else { // normal mode

            if (isdigit(command)) {
                commandCount = command - '0';
                continue;
            }

            switch (command) {
            case 'i': // toggle to insert mode
                searching = false;
                editor.toggleMode();
                firstPressed = '*';
                break;
            case 38: // Up
                editor.moveUpPerfect();
                firstPressed = '*';
                break;
            case 40: // Down
                editor.moveDownPerfect();
                firstPressed = '*';
                break;
            case 37: // Left
                editor.moveLeft();
                firstPressed = '*';
                break;
            case 39: // Right
                editor.moveRight();
                firstPressed = '*';
                break;
            case 'd': // delete line (dd)
                if (firstPressed == 'd') {
                    if (commandCount == 1) {
                        editor.deleteCurrentLine();
                    } else {
                        editor.executeWithCount(commandCount, "dd");
                    }
                    firstPressed = '*';
                } else {
                    firstPressed = 'd'; // Wait for second 'd'
                }
                break;
            case 'D': // delete from cursor to end of line
                editor.deleteToEndOfLine();
                firstPressed = '*';
                break;
            case 'x': // delete character at cursor
                editor.deleteCharacterAtCursor();
                firstPressed = '*';
                break;
            case 'y': // copy line (yy)
                if (firstPressed == 'y') {
                    if (commandCount == 1) {
                        editor.yankLine();
                    } else {
                        editor.executeWithCount(commandCount, "yy");
                    }
                    firstPressed = '*';
                } else {
                    firstPressed = 'y'; // Wait for second 'y'
                }
                break;
            case 'p': // Paste after line
                editor.pasteAfter();
                firstPressed = '*';
                break;
            case 'P': // Paste before line
                editor.pasteBefore();
                firstPressed = '*';
                break;
            case '0': //  start of line
                editor.moveToStartOfLine();
                firstPressed = '*';
                break;
            case '$': //  end of line
                editor.moveToEndOfLine();
                firstPressed = '*';
                break;
            case 'w': // start of next word
                editor.moveToNextWord();
                firstPressed = '*';
                break;
            case 'b': // start of previous word
                editor.moveToPreviousWord();
                firstPressed = '*';
                break;
            case 'e': // end of current/next word
                editor.moveToWordEnd();
                firstPressed = '*';
                break;
            case 8: // backspace
                editor.delChar();
                firstPressed = '*';
                break;
            case 'J':
                editor.joinLineWithNext();
                firstPressed = '*';
                break;
            case 'j':
                if (commandCount == 1) {
                    editor.moveDown();
                } else {
                    editor.executeWithCount(commandCount, "j");
                }
                break;
            case 'k':
                if (commandCount == 1) {
                    editor.moveUp();
                } else {
                    editor.executeWithCount(commandCount, "k");
                }
                break;
            case ':':
                editor.display(':', ":");
                char initialGetch;
                initialGetch = _getch();
                if (initialGetch == 'd') {
                    editor.display('d', ":d");
                    command = _getch();
                    if (isdigit(command)) {
                        commandCount = command - '0'; // Convert char to int
                        editor.deleteLine(commandCount);
                        editor.display('d', ":d " + to_string(commandCount)); // Delete N lines
                        missDisplay = true;
                    }
                } else if (initialGetch == 'w') {
                    editor.display('w', ":w ");
                    string cmd = ":w ";
                    string file = "";
                    string i;
                    cin >> i;
                    file += i;
                    cmd += i;
                    editor.display('w', cmd + '|');
                    editor.saveToFile(file);
                    Sleep(500);
                } else if (initialGetch == 'e') {
                    editor.display('e', ":e ");
                    string cmd = ": ";
                    string file = "";
                    string i;
                    cin >> i;
                    file += i;
                    cmd += i;
                    editor.display('e', cmd + '|');
                    editor.openFile(file);
                    Sleep(500);
                } else if (initialGetch == 'q') {
                    editor.display('q', ":q ");
                    command = _getch();
                    if (command == '!') {
                        editor.display('q', ":q! ");
                        return 0;
                    } else {
                        editor.display('q', ":q ", "Do you want to quit without saving?\nSave:(y/n)");
                        if (_getch() == 'y') {
                            editor.saveToFile("Save.txt");
                            Sleep(500);
                        } else {
                            return 0;
                        }

                    }
                } else if (initialGetch == 's') {
                    editor.display('s', ":s ");
                    command = _getch();
                    if (command == '/') {
                        editor.findAndDisplay('/');
                        editor.replaceFindText();
                    }

                } else {
                    editor.display(initialGetch, ":", "Error: Wrong Command");
                    Sleep(1000);
                }

                break;
            case '>':
                if (firstPressed == '>') {
                    editor.indentLine();
                    firstPressed = '*';
                } else {
                    firstPressed = '>';
                }
                break;
            case '<':
                if (firstPressed == '<') {
                    editor.unindentLine();
                    firstPressed = '*';
                } else {
                    firstPressed = '<';
                }
                break;
            case '/':
                searching = true;
                editor.findAndDisplay(13);
                break;
            case 'n':
                if (searching) {
                    editor.moveToNextSearch();
                }
                break;
            case 'N':
                if (searching) {
                    editor.moveToPrevSearch();
                }
                break;
            default:
                editor.display(initialGetch, ":", "Error: Invalid Command");
                Sleep(1000);
                firstPressed = '*';
                break;
            }
        }
    }
}    return 0;
