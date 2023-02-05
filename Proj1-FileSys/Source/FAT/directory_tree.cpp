#include "util.h"

DirectoryTree::DirectoryTree() {}

DirectoryTree::DirectoryTree(FAT32* FAT)
{
    this->init(FAT);
}

DirectoryTree::~DirectoryTree() {}

void DirectoryTree::init(FAT32* FAT) {
    this->FAT = FAT;

    this->clusterSize = FAT->SectorPerCluster * FAT->BytePerSector;
    // this->clusterSize = BS->BPB_SecPerClus * BS->BPB_BytsPerSec;
    this->EntryPerCluster = this->clusterSize / 32;
    this->rootCluster = 2;
}

vector<Element> DirectoryTree::readDirectoryTree(DWORD cluster) {
    this->firstCluster = cluster;

    DWORD curCluster = this->firstCluster;
    ULONGLONG clusterOffset = 0;
    BYTE* clusterBuffer = new BYTE[clusterSize];
    memset(clusterBuffer, 0, clusterSize);

    vector<Element> list;
    do {
        clusterOffset = ((curCluster - 2) * FAT->SectorPerCluster + FAT->FirstDataSector) * FAT->BytePerSector;
        LoadDisk(FAT->sPath.c_str(), clusterOffset, clusterBuffer, clusterSize);
        BYTE Entry[32];
        memset(Entry, 0, 32);
        wstring longName;
        
        for (int i = 0; i < EntryPerCluster; i++)
        {
            readToArr(Entry, clusterBuffer, 32*i, 32);
            if (Entry[0] != 0x00 && Entry[0] != 0xE5 && Entry[0] != 0x20 && Entry[0] != 0x2E) {
                if (Entry[11] == 0x10 || Entry[11] == 0x20) {
                    Element element;
                    element.name = readToString(Entry, 0, 8);
                    element.name = trim(element.name);
                    element.expandName = readToString(Entry, 8, 3);
                    element.expandName = trim(element.expandName);
                    if (longName.length()) {
                        element.longName = trim(longName);
                    }
                    else {
                        string name = element.name;
                        if (!element.expandName.empty()) {
                            name += "." + element.expandName;
                        }
                        for (char c: name) {
                            element.longName += tolower(c);
                        }
                    }

                    longName = L"";

                    element.size = readToNumber(Entry, 28, 4);
                    element.firstCluster = readToNumber(Entry, 26, 2);
                    if (Entry[11] == 0x10) {
                        element.type = 'D';
                    }
                    else {
                        element.type = 'A';
                    }
                    list.push_back(element);
                
                }
                if (Entry[11] == 0x0F) {
                    longName = readExtraEntry(Entry) + longName;
                }
                else {
                    longName = L"";
                }
            }
            else if (Entry[0] == 0x2E && Entry[1] == 0x2E) {
                this->rootCluster = readToNumber(Entry, 26, 2);
                if (this->rootCluster <= 2) {
                    this->rootCluster = 2;
                }
            }
        }
        
        curCluster = FAT->nextClusterOf(curCluster);
        if (curCluster == 0x00000000 || curCluster == 0xFFFFFFF7 || curCluster == 0x0FFFFFFF || (curCluster >= 0xFFFFFFF8 && curCluster <= 0xFFFFFFFF)) {
            break;
        }
        
    } while (true);

    delete[] clusterBuffer;
    this->children = list;
    return this->children;
}

void DirectoryTree::printList() {
    showDetails(this->children);
}

void DirectoryTree::readTxtFile(wstring name) {
    _setmode(_fileno(stdout), _O_TEXT);
    DWORD curCluster = this->findFile(name);
    if (curCluster == 0) {
        wcout << L"File not found." << endl;
        return;
    }

    ULONGLONG clusterOffset = 0;
    BYTE* clusterBuffer = new BYTE[clusterSize + 1];
    memset(clusterBuffer, 0, clusterSize);

    do {
        clusterOffset = ((curCluster - 2) * FAT->SectorPerCluster + FAT->FirstDataSector) * FAT->BytePerSector;
        LoadDisk(FAT->sPath.c_str(), clusterOffset, clusterBuffer, clusterSize);

        printf("%s", clusterBuffer);

        curCluster = FAT->nextClusterOf(curCluster);
        if (curCluster == 0x00000000 || curCluster == 0xFFFFFFF7 || curCluster == 0x0FFFFFFF ||(curCluster >= 0xFFFFFFF8 && curCluster <= 0xFFFFFFFF)) {
            break;
        }
    } while (true);
    delete[] clusterBuffer;
    cout << endl;
    _setmode(_fileno(stdout), _O_WTEXT);
}

DWORD DirectoryTree::findChild(wstring name) {
    for (Element child : this->children) {
        if (child.type == 'D' && name.length() == child.longName.length()) {
            int len = name.length();
            bool isEqual = true;
            for (int i = 0; i < len; i++) {
                wchar_t c1 = name[i];
                wchar_t c2 = child.longName[i];
                if (c1 != c2) { 
                    isEqual = false;
                    break;
                }
            }
            if (isEqual) return child.firstCluster;
        }
    }
    return 0;
}

DWORD DirectoryTree::findFile(wstring name) {
    for (Element child : this->children) {
        if (child.type == 'A' && name.length() == child.longName.length()) {
            int len = name.length();
            bool isEqual = true;
            for (int i = 0; i < len; i++) {
                wchar_t c1 = name[i];
                wchar_t c2 = child.longName[i];
                if (c1 != c2) { 
                    isEqual = false;
                    break;
                }
            }
            if (isEqual) return child.firstCluster;
        }
    }
    return 0;
}