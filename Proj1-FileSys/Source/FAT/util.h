/*
* This file contains common libraries and base functions.
* Include all libraries you need here.
*/
#ifndef UTIL_H_
#define UTIL_H_

#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <exception>
#include <algorithm>
#include <fcntl.h>
#include <io.h>   
#include <sstream>
#include <Windows.h>
#include <iomanip>
using namespace std;

class BootSector
{
public:
    string  sPath;
    BYTE    BS_jmpBoot[3];
    BYTE    BS_OEMName[8];
    WORD    BPB_BytsPerSec;
    BYTE    BPB_SecPerClus;
    WORD    BPB_RsvdSecCnt;
    BYTE    BPB_NumFATs;
    WORD    BPB_RootEntCnt;
    WORD    BPB_TotSec16;
    BYTE    BPB_Media;
    WORD    BPB_FATSz16;
    WORD    BPB_SecPerTrk;
    WORD    BPB_NumHeads;
    DWORD   BPB_HiddSec;
    DWORD   BPB_TotSec32;
    DWORD   BPB_FATSz32;
    WORD    BPB_ExFlags;
    WORD    BPB_FS_Ver;
    DWORD   BPB_RootClus;
    WORD    BPB_FSInfo;
    WORD    BPB_BkBootSec;
    BYTE    BPB_Reserved[12];
    BYTE    BS_DrvNum;
    BYTE    BS_Reserved1;
    BYTE    BS_BootSig;
    DWORD   BS_VolID;
    BYTE    BS_VolLab[11];
    BYTE    BS_FilSysType[8];
    BYTE    BootCode[420];
    BYTE    EndCode[2];


public:
    BootSector();
    BootSector(string path);
    ~BootSector();
    int load(string path);
    void print();

    long long FirstDataSector();
    long long FirstFAT1Sector();
    long long FirstFAT2Sector();

    string getPath();
};

class FAT32
{
public:
    int EntryPerSector;
    int BytePerSector;
    int SectorPerCluster;
    ULONGLONG FirstDataSector;
    string sPath;
    ULONGLONG FirstFAT1Sector;
    ULONGLONG FirstFAT2Sector;
    BYTE* cache;
    ULONGLONG sector_cache;

    void loadCache(long long newSectorCache);
public:
    FAT32();
    FAT32(BootSector* BS);
    ~FAT32();
    void load(BootSector* BS);
    void printCache();

    DWORD nextClusterOf(DWORD curCluster);
};

typedef struct Element {
    string name;
    string expandName;
    wstring longName;
    DWORD size;
    char type;
    WORD firstCluster;
} Element;


enum Color { LightBlue = 9, Yellow = 6, Green = 2, White = 7};

bool compareName(Element a, Element b) {
	return a.name < b.name;
}

void showHeader(int color) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, color);
	wcout << setw(50) << left << L"Name";
	wcout << setw(15) << left << L"Size";
	wcout << setw(5) << left << L"Type";
	wcout << setw(15) << right << L"firstCluster" << endl;
	// Show line space
	SetConsoleTextAttribute(handle, White);
	wcout << setfill(L'-');
	wcout << setw(85) << L"-" << endl;
	wcout << setfill(L' ');
}

void showContent(Element e, int color) {
	// Set text color
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, color);
	wcout << setw(50) << left << e.longName;
	wcout << setw(15) << left << e.size;
	wcout << setw(5) << left << e.type;
	wcout << setw(15) << right << e.firstCluster << endl;
	// Reset text color
	SetConsoleTextAttribute(handle, White);
}

void showDetails(vector<Element> Elements) {
	// Sort array's name before show details
	vector<Element> typeA;
	vector<Element> typeD;
	for (auto& i : Elements)
		if (i.type == 'A')
			typeA.push_back(i);
		else
			typeD.push_back(i);
	sort(typeD.begin(), typeD.end(), compareName);
	sort(typeA.begin(), typeA.end(), compareName);
	// Show header
	showHeader(LightBlue);
	// Show content
	for (auto& i : typeD) {
		showContent(i, Yellow);
	}
	for (auto& i : typeA) {
		showContent(i, Green);
	}	
}

class DirectoryTree
{
public:
    FAT32* FAT;
    int clusterSize;
    int EntryPerCluster;
    DWORD firstCluster;
    DWORD rootCluster;
    vector<Element> children;
    DWORD findChild(wstring name);
    DWORD findFile(wstring name);
public:
    DirectoryTree();
    DirectoryTree(FAT32* FAT);
    ~DirectoryTree();

    void init(FAT32* FAT);
    vector<Element> readDirectoryTree(DWORD cluster);
    void printList();
    void readTxtFile(wstring name);
};




/**
 * Read particular data from the disk.
 *
 * @param[in] drive wchar disk path
 * @param[in] offset read point in byte 
 * @param[out] buffer reading data
 * @param[in] size size of data to read
 * 
 * @return 1 if can not open file, 2 if can not read data, 0 otherwise
 */
int LoadDisk(const char* path, ULONGLONG offset, BYTE *buffer, INT size)
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFileA(path,                              // Drive to open
                        GENERIC_READ,                       // Access mode
                        FILE_SHARE_READ | FILE_SHARE_WRITE, // Share Mode
                        NULL,                               // Security Descriptor
                        OPEN_EXISTING,                      // How to create
                        0,                                  // File attributes
                        NULL);                              // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        printf("Error in Create File: %u\n", GetLastError());
        return 1;
    }

    LONG h_off = offset >> 32;
    SetFilePointer(device, offset, NULL, FILE_BEGIN); // Set a Point to Read

    if (!ReadFile(device, buffer, size, &bytesRead, NULL))
    {
        printf("Error in Read File: %u\n", GetLastError());
        return 2;
    }
    else
    {
        return 0;
    }
}

/**
 * find nth root directory of the passed path
 *
 * @param[in] path current path
 * @param[in] generation level of root directory
 * 
 * @return root path if found, "" otherwise
 */
std::string findAncestorPath(std::string path, int generation)
{
    for (int i = 0; i < generation; i++)
    {
        std::size_t found = path.find_last_of("/\\");
        if (found == -1) {
            return "";
        }
        path = path.substr(0,found);
    }
    return path;
}

/**
 * read number from BYTE array
 *
 * @param[in] buffer array to read
 * @param[in] offset offset to start reading from buffer
 * @param[in] len number of bytes to read
 * 
 * @return number read from buffer
 */
unsigned long long readToNumber(BYTE* buffer, int offset, int len) {
    long long result = 0;
    memcpy(&result, buffer + offset, len);
    return result;
}

/**
 * read string from BYTE array
 *
 * @param[in] buffer array to read
 * @param[in] offset offset to start reading from buffer
 * @param[in] len number of bytes to read
 * 
 * @return std::sting read from buffer
 */
std::string readToString(BYTE* buffer, const int offset, const int len) {
    string result = "";
    for (int i = 0; i < len; i++) {
        result += buffer[i+offset];
    }
    return result;
}

/**
 * read wstring from BYTE array
 *
 * @param[in] buffer array to read
 * @param[in] offset offset to start reading from buffer
 * @param[in] len number of bytes to read
 * 
 * @return std::wstring read from buffer
 */
std::wstring readToStringUnicode(BYTE* buffer, const int offset, const int len) {
    wstring result;
    for (int i = 0; i < len; i += 2) {
        wchar_t c;
        memcpy(&c, buffer + offset + i, 2);
        if (c != 0xFFFF && c!= 0x0000)
            result += c;
    }
    return result;
}

/**
 * copy BYTE array from BYTE array
 *
 * @param[out] dest destination buffer
 * @param[in] buffer source buffer
 * @param[in] offset offset to start reading from buffer
 * @param[in] len number of bytes to read
 * 
 */
void readToArr(BYTE *dest, const BYTE *buffer, const int offset, const int len) {
    for (int i = 0; i < len; i++) {
        dest[i] = buffer[i+offset];
    }
}

void printByteASCII(BYTE * str, const int len) {
    wstring s;
    for (int i = 0; i < len; i++) {
        s+=str[i];
        if (i%16 == 15) {
            s+='\n';
        }
    }
    wcout << s << endl;
}

void printByteHex(BYTE * buffer, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02X ", buffer[i]);
        if (i%16 == 15) {
            printf("\n");
        }
    }
    printf("\n");
}


const std::string WHITESPACE = " \n\r\t\f\v\0";
const std::wstring W_WHITESPACE = L" \n\r\t\f\v\0";

std::string ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string &s) {
    return rtrim(ltrim(s));
}

std::wstring ltrim(const std::wstring &ws)
{
    size_t start = ws.find_first_not_of(W_WHITESPACE);
    return (start == std::string::npos) ? L"" : ws.substr(start);
}

std::wstring rtrim(const std::wstring &ws)
{
    size_t end = ws.find_last_not_of(W_WHITESPACE);
    return (end == std::string::npos) ? L"" : ws.substr(0, end + 1);
}

std::wstring trim(const std::wstring &ws) {
    return rtrim(ltrim(ws));
}

wstring readExtraEntry(BYTE* buffer) {
    wstring name;
    name = readToStringUnicode(buffer, 1, 10);
    name += readToStringUnicode(buffer, 14, 12);
    name += readToStringUnicode(buffer, 28, 4);
    return name;

}


#endif