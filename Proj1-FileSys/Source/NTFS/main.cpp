#include "util.h"

#define SECTOR_SIZE 512
#define MFT_ENTRY_SIZE 1024

// Disk path
string Disk("");
// NTFS_BPB variables
WORD SectorSize(0);
DWORD ClusterSize(0);
DWORD FileRecordSize(0);
DWORD IndexBlockSize(0);
ULONGLONG MFTAddr(0);
// program global variables
vector<Element> eleList;
vector<wstring> dir;
vector<DWORD> rootID;

// read user input disk
void readInput() {
    wstring input;
    wcout << "Enter Disk to open: ";
    Disk = "\\\\.\\";
    wcin >> input;
    for (char c: input) {
        Disk+= c;
    }
    Disk += ":";
    dir.push_back(input+L":");
}

// read NTFS BPB, get NTFS_BPB global variables
int read_NTFS_BPB(string Disk) {
    // master boot sector
    NTFS_BPB mbs;
    if (LoadDisk(Disk.c_str(), 0, (BYTE*)&mbs, SECTOR_SIZE)) {
        wcout << L"Can not read boot sector\n";
        system("pause");
        return 1;
    }

    // verify signature
    if (strncmp(NTFS_SIGNATURE, (const char*)mbs.Signature, 8)) {
        wcout << L"Not NTFS\n";
        system("pause");
        return 1;
    }

    // get some useful BPB variables
    _setmode(_fileno(stdout), _O_TEXT);
    SectorSize = mbs.BytesPerSector;
    printf("Sector size: \t\t\t%u bytes\n", SectorSize);
    ClusterSize = SectorSize * mbs.SectorsPerCluster;
    printf("Cluster size: \t\t\t%u bytes\n", ClusterSize);

    int sz = (char)mbs.ClustersPerFileRecord;
    if (sz > 0)
        FileRecordSize = ClusterSize * sz;
    else
        FileRecordSize = 1 << (-sz);
    printf("File Record size: \t\t%u bytes\n", FileRecordSize);

    sz = (char)mbs.ClustersPerIndexBlock;
    if (sz > 0)
        IndexBlockSize = ClusterSize * sz;
    else
        IndexBlockSize = 1 << (-sz);
    printf("Index Block size: \t\t%u bytes\n", IndexBlockSize);

    MFTAddr = mbs.LCN_MFT * ClusterSize;
    printf("MFT Address: \t\t\t0x%016I64X\n", MFTAddr);
    _setmode(_fileno(stdout), _O_WTEXT);
    return 0;
}

// read a $MFT Entry to an Element
Element readMFTEntry(ULONGLONG MFTOffset, bool&isFailed, DWORD curID) {

    BYTE* MFT_Entry = new BYTE[MFT_ENTRY_SIZE]; 
    LoadDisk(Disk.c_str(), MFTOffset, MFT_Entry, 1024);
    // file record header (first 48 bytes)
    FILE_RECORD_HEADER* frh = (FILE_RECORD_HEADER*)MFT_Entry;

    // element to return
    Element element;
    element.name = L"";
    element.size = 0;
    element.type = 0;
    element.ID = curID;
    element.parentID = 0;
    element.dataOffset = 0ULL;

    // check Magic
    if (frh->Magic == FILE_RECORD_MAGIC)
    {
        // check file record header flag
        if (frh->Flags == FILE_RECORD_FLAG_INUSE || frh->Flags == FILE_RECORD_FLAG_DIR) {
            // determine element type
            if (frh->Flags == FILE_RECORD_FLAG_DIR) {
                element.type = 'D';
            }
            else {
                element.type = 'A';
            }
            // attribute header common and its offset
            WORD attrOff = frh->OffsetOfAttr;
            ATTR_HEADER_COMMON* ahc = (ATTR_HEADER_COMMON*)(MFT_Entry + attrOff);

            // read each attributes if they are accessible
            bool accessible = false;
            while (ahc->Type != 0xFFFFFFFF) {
                // attribute $STANDARD_INFORMATION
                if (ahc->Type == ATTR_TYPE_STANDARD_INFORMATION) {
                    // attribute is resident
                    if (!ahc->NonResident) {
                        // standard information header resident
                        ATTR_HEADER_RESIDENT* sihr = (ATTR_HEADER_RESIDENT*)((BYTE*)ahc);
                        // attribute standard information
                        ATTR_STANDARD_INFORMATION* asi = (ATTR_STANDARD_INFORMATION*)((BYTE*)ahc+sizeof(ATTR_HEADER_RESIDENT));
                        // check permissions
                        if (asi->Permission == 0 || asi->Permission == ATTR_STDINFO_PERMISSION_READONLY || asi->Permission == ATTR_STDINFO_PERMISSION_ARCHIVE || asi->Permission == ATTR_STDINFO_PERMISSION_DEVICE || asi->Permission == ATTR_STDINFO_PERMISSION_NORMAL || asi->Permission == ATTR_STDINFO_PERMISSION_SPARSE) {
                            accessible = true;
                        }
                        else {
                            accessible = false;
                        }
                    }
                }
                // attribute $FILE_NAME
                else if (ahc->Type == ATTR_TYPE_FILE_NAME) {
                    // attribute is resident 
                    if (!ahc->NonResident) {
                        // file name header resident
                        ATTR_HEADER_RESIDENT* fnhr = (ATTR_HEADER_RESIDENT*)((BYTE*)ahc);
                        // attribute file name
                        ATTR_FILE_NAME* afn = (ATTR_FILE_NAME*)((BYTE*)ahc+sizeof(ATTR_HEADER_RESIDENT));
                        wstring file_name = readToStringUnicode(MFT_Entry, attrOff + 90, 2*afn->NameLength);
                        // parse to element name and parentID
                        element.name =  file_name;
                        element.parentID = afn->ParentRef;
                        if (element.parentID <= 24) {
                            element.parentID = 0;
                        }
                    }
                }
                // attribute $DATA
                else if(ahc->Type == ATTR_TYPE_DATA && element.size == 0ULL) {
                    // attribute is resident
                    if (!ahc->NonResident) {
                        // data header resident
                        ATTR_HEADER_RESIDENT* dhr = (ATTR_HEADER_RESIDENT*)((BYTE*)ahc);
                        // parse to element size and data offset
                        element.size = dhr->AttrSize;
                        element.dataOffset = MFTOffset + attrOff + dhr->AttrOffset; 
                    }
                    // attribute is non resident
                    else {
                        // data herder non resident
                        ATTR_HEADER_NON_RESIDENT* dhnr = (ATTR_HEADER_NON_RESIDENT*)((BYTE*)ahc);
                        // decode the first data cluster
                        BYTE chainLen = readToNumber(MFT_Entry, attrOff + 0x40, 1)/0x10;
                        BYTE chainOff = readToNumber(MFT_Entry, attrOff + 0x40, 1)%0x10;
                        DWORD fDataClus = readToNumber(MFT_Entry, attrOff + 0x41 + chainOff, chainLen);
                        // parse to element size and data offset
                        element.size = dhnr->RealSize;
                        element.dataOffset = fDataClus * ClusterSize;
                    }
                }
                // this $MFT is not accessible
                if (!accessible) {
                    break;
                }
                // jump to next attribute
                attrOff += ahc->TotalSize;
                ahc = (ATTR_HEADER_COMMON*)(MFT_Entry + attrOff);
            }
        }
        // parse element completed
        if (!element.name.empty()) {
            isFailed = false;
            delete[] MFT_Entry;
            return element;
        }
    }
    // parse element failed
    isFailed = true;
    delete[] MFT_Entry;
    return element;
}

// compare two elements by their parentID
bool compareByParentID(const Element &a, const Element &b)
{
    return a.parentID < b.parentID;
}

// read Master File Table
void readMFT() {
    // first $MFT
    ULONGLONG MFTOff = MFTAddr;
    DWORD ID = 0;
    // gap to detect end of MFT (1024)
    bool isFailed = false;
    int failedCnt = 0;
    while (failedCnt < 1024)
    {
        Element element = readMFTEntry(MFTOff, isFailed, ID);
        if (isFailed) {
            failedCnt++;
        }
        else { 
            failedCnt = 0;
            eleList.push_back(element);
        }
        // next $MFT
        ID++;
        MFTOff += 1024;
    }
    // sort by name
    std::sort(eleList.begin(), eleList.end(), compareByParentID);
}

int goBack() {
    DWORD backID = 0;
    if (rootID.size() > 1) {
        rootID.pop_back();
        backID = rootID[rootID.size() - 1];
        vector<Element> lst;
        for (auto& ele : eleList) {
            if (ele.parentID == backID) {
                lst.push_back(ele);
            }
        }
        showDetails(lst);
        return 1;
    }
    return 0;
}

int goForward(wstring directory) {
    
    if (directory == L".") {
        vector<Element> lst;
        DWORD curID = rootID[rootID.size() - 1];
        wcout << directory << endl;
        for (auto& ele : eleList) {
            if (ele.parentID == curID) {
                lst.push_back(ele);
            }
        }
        showDetails(lst);
        return 1;
    }
    DWORD parentID = -1;
    for (auto& ele : eleList) {
        if (ele.name == directory && ele.type == 'D') {
            parentID = ele.ID;
            break;
        }
    }
    if(parentID != -1) {
        rootID.push_back(parentID);
        vector<Element> lst;
        wcout << directory << endl;
        for (auto& ele : eleList) {
            if (ele.parentID == parentID) {
                lst.push_back(ele);
            }
        }
        showDetails(lst);
        dir.push_back(directory);
        return 1;
    }
    wcout << L"Directory not found\n";
    return 0;
}

void printCommandList() {
    wcout << L"************************************\n";
    wcout << L"cd <dir>   : go to <dir>\n";
    wcout << L"cd ..      : go to parent directory\n";
    wcout << L"open <file>: catch .txt file or open other file\n";
    wcout << L"quit       : quit the program\n";
    wcout << L"************************************\n";
}

int openTxt(wstring filename) {
    _setmode(_fileno(stdout), _O_TEXT);
    DWORD parentID = rootID.back();
    for (auto& ele : eleList) {
        if (ele.parentID == parentID && ele.name == filename) {
            if (ele.dataOffset != 0) {
                BYTE* dataEntry = new BYTE[MFT_ENTRY_SIZE + 1];
                memset(dataEntry, 0, MFT_ENTRY_SIZE + 1);
                ULONGLONG entryOffset = MFT_ENTRY_SIZE * (ele.dataOffset / MFT_ENTRY_SIZE);

                WORD readLen;
                readLen = min(ele.dataOffset + MFT_ENTRY_SIZE - entryOffset, ele.size);

                LoadDisk(Disk.c_str(), entryOffset, dataEntry, MFT_ENTRY_SIZE);
                printf("%.*s", readLen, dataEntry + (ele.dataOffset - entryOffset));

                if (readLen < ele.size) {
                    printf(" [.. +%llu characters]", ele.size - readLen);
                }

                printf("\n");
                _setmode(_fileno(stdout), _O_WTEXT);
                delete[] dataEntry;
                return 1;
            }
        }
    }
    _setmode(_fileno(stdout), _O_WTEXT);
    return 0;
}

int readCommand() {
    // print current directory
    wstring input;
    for (wstring directory: dir) {
        wcout << directory << L"\\";
    }
    wcout << L"> ";
    // read input command as a wstring
    do {
        getline(wcin, input, L'\n');
    } while (input.length() <= 2);
    wcin.ignore(1000, '\n');

    // split input into the commands vector
    vector<wstring> commands;
    wstring command = L"";
    bool isContinuous = false;
    for (auto c : input)
    {
        // detect ""
        if (c == L'"') {
            isContinuous = !isContinuous;
            continue;
        }
        else if (c == L' ' && !isContinuous)
        {
            commands.push_back(command);
            command = L"";
        }
        else {
            command = command + c;
        }
    }
    commands.push_back(command);

    // command cd (change dir)
    if (commands[0] == L"cd") {
        // go back to the parent directory
        if (commands[1] == L"..") {
            if (goBack() && dir.size() > 1) {
                dir.pop_back();
            };
        }
        // go forward to the specific child directory
        else {
            goForward(commands[1]);
        }
    }
    // command open file
    else if (commands[0] == L"open") {
        // catch txt file to console
        if (commands[1].substr(commands[1].size() - 4) == L".txt") {
            openTxt(commands[1]);
        }
        // open other files with compatable application
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
    // quit command 
    else if (commands[0] == L"quit") {
        return 0;
    }
    // cmdlst command to show command list
    else if (commands[0] == L"cmdlst") {
        printCommandList();
    }
    // command is not available
    else {
        wcout << L"Command not exist.\n";
        wcout << L"Type 'cmdlst' to show available commands.\n";
    }
    return 1;
}
int main() {
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stdin), _O_WTEXT);

    readInput();
    if (read_NTFS_BPB(Disk)) {
        return 1;
    }

    Element rootEle;
    rootEle.name = L".";
    rootEle.size = 0;
    rootEle.type = 'D';
    rootEle.ID = 0;
    rootEle.parentID = 0xFFFFFFFF;
    eleList.push_back(rootEle);
    readMFT();
    rootID.push_back(0);
    goForward(L".");
    
    while (readCommand());
    
    system("pause");
    return 0;
}