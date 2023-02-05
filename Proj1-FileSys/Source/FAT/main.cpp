#include "util.h"
#include "FAT32.cpp"
#include "boot_sector.cpp"
#include "directory_tree.cpp"


vector<wstring> dir;
BootSector BS;
FAT32 FAT;
DirectoryTree DT;

void printCommandList() {
    wcout << L"************************************\n";
    wcout << L"cd <dir>   : go to <dir>\n";
    wcout << L"cd ..      : go to parent directory\n";
    wcout << L"open <file>: catch .txt file or open other file\n";
    wcout << L"quit       : quit the program\n";
    wcout << L"************************************\n";
}
string readInput(wstring& RootDir) {
    wcout << "Enter Disk to open: ";
    wcin >> RootDir;
    RootDir += L":";

    string disk = "\\\\.\\";
    for (char c : RootDir) {
        disk += c;
    }
    return disk;
}

int goBack() {
    WORD backCluster = DT.rootCluster;
    DT.readDirectoryTree(backCluster);
    system("cls");
    DT.printList();
    if (backCluster < 2) {
        return 0;
    }
    else {
        return 1;
    }
}

int goForward(wstring directory) {
    WORD forwardCluster = DT.findChild(directory);
    if (forwardCluster == 0) {
        wcout << L"Directory not found." << endl;
        return 0;
    }
    else {
        DT.readDirectoryTree(forwardCluster);
        system("cls");
        DT.printList();
        return 1;
    }
}
 
int openTxt(wstring fName) {
    DT.readTxtFile(fName);
    return 0;
}

int readCommand() {
    wstring input;
    for (wstring directory: dir) {
        wcout << directory << L"\\";
    }
    wcout << L"> ";

    do {
        getline(wcin, input, L'\n');
    } while (input.length() <= 2);
    wcin.ignore(1000, '\n');

    wistringstream wiss(input);
    vector<wstring> commands;
    wstring command = L"";
    bool isContinous = false;
    for (auto c : input)
    {
        if (c == L'"') {
            isContinous = !isContinous;
            continue;
        }
        else if (c == L' ' && !isContinous)
        {
            commands.push_back(command);
            command = L"";
        }
        else {
            command = command + c;
        }
    }
    commands.push_back(command);

    if (commands[0] == L"cd") {
        if (commands[1] == L"..") {
            if (goBack() && dir.size() > 1) {
                dir.pop_back();
            };
        }
        else {
            if (goForward(commands[1])) {
                dir.push_back(commands[1]);
            }
        }
    }
    else if (commands[0] == L"open") {
        if (commands[1].substr(commands[1].size() - 4) == L".txt") {
            openTxt(commands[1]);
        }
        else {
            wstring fileName;
            for (auto ws: dir) {
                fileName += ws + L'\\';
            }
            fileName += commands[1];

            string sFileName;
            for (auto c: fileName)
            {
                sFileName += c;
            }
            system(sFileName.c_str());
        }
    }
    else if (commands[0] == L"quit") {
        return 0;
    }
    else if (commands[0] == L"cmdlst") {
        printCommandList();
    }
    else {
        wcout << L"Command not exist.\n";
        wcout << L"Type 'cmdlst' to show available commands.\n";
    }
    return 1;
}
int main(int argc, char*argv[]) {
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stdin), _O_WTEXT);

    wstring RootDir = L"";
    string disk = readInput(RootDir); 
    int BS_flag = BS.load(disk); 
    if (BS_flag == 1 || BS_flag == 3) {
        wcout << L"This disk is unreadable or not exist.\n";
        system("pause");
        return 1;
    }
    else if (BS_flag == 2) { 
        wcout << L"This disk is not formatted as FAT32.\n";
        system("pause");
        return 1;
    }
    else if (BS_flag != 0) {
        wcout << L"Something went wrong while reading this disk.\n";
        system("pause");
        return 1;
    }
    BS.print();
    FAT.load(&BS);
    DT.init(&FAT);

    dir.push_back(RootDir);
    DT.readDirectoryTree(2);
    DT.printList();

    while (readCommand());
    return 0;
    
}
