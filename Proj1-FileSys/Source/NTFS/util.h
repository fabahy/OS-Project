/*
 * NTFS util files
 * 
 * Copyright(C) 2010 cyb70289 <cyb70289@gmail.com>
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

#define	NTFS_SIGNATURE		"NTFS    "

#pragma pack(1)
typedef struct tagNTFS_BPB
{
    BYTE		Jmp[3];
	BYTE		Signature[8];
	WORD		BytesPerSector;
	BYTE		SectorsPerCluster;
	WORD		ReservedSectors;
	BYTE		Zeros1[3];
	WORD		NotUsed1;
	BYTE		MediaDescriptor;
	WORD		Zeros2;
	WORD		SectorsPerTrack;
	WORD		NumberOfHeads;
	DWORD		HiddenSectors;
	DWORD		NotUsed2;
	DWORD		NotUsed3;
	ULONGLONG	TotalSectors;
	ULONGLONG	LCN_MFT;
	ULONGLONG	LCN_MFTMirr;
	DWORD		ClustersPerFileRecord;
	DWORD		ClustersPerIndexBlock;
	BYTE		VolumeSN[8];
	BYTE		Code[430];
	BYTE		EndCode[2];
} NTFS_BPB;

// MFT Indexes
#define	MFT_IDX_MFT				0
#define	MFT_IDX_MFT_MIRR		1
#define	MFT_IDX_LOG_FILE		2
#define	MFT_IDX_VOLUME			3
#define	MFT_IDX_ATTR_DEF		4
#define	MFT_IDX_ROOT			5
#define	MFT_IDX_BITMAP			6
#define	MFT_IDX_BOOT			7
#define	MFT_IDX_BAD_CLUSTER		8
#define	MFT_IDX_SECURE			9
#define	MFT_IDX_UPCASE			10
#define	MFT_IDX_EXTEND			11
#define	MFT_IDX_RESERVED12		12
#define	MFT_IDX_RESERVED13		13
#define	MFT_IDX_RESERVED14		14
#define	MFT_IDX_RESERVED15		15
#define	MFT_IDX_USER			16

// File Record Header
#define	FILE_RECORD_MAGIC		1162627398
#define	FILE_RECORD_FLAG_INUSE	0x01	// File record is in use
#define	FILE_NAME_INDEX_PRESENT	0x02	// 
#define	FILE_RECORD_FLAG_DIR	0x03	// File record is a directory

typedef struct tagFILE_RECORD_HEADER
{
	DWORD		Magic;			// 0x00	"FILE"
	WORD		OffsetOfUS;		// 0x04	Offset of Update Sequence
	WORD		SizeOfUS;		// 0x06	Size in words of Update Sequence Number & Array
	ULONGLONG	LSN;			// 0x08	$LogFile Sequence Number
	WORD		SeqNo;			// 0x10	Sequence number
	WORD		Hardlinks;		// 0x12	Hard link count
	WORD		OffsetOfAttr;	// 0x14	Offset of the first Attribute
	WORD		Flags;			// 0x16	Flags
	DWORD		RealSize;		// 0x18	Real size of the FILE record
	DWORD		AllocSize;		// 0x1C	Allocated size of the FILE record
	ULONGLONG	RefToBase;		// 0x20	File reference to the base FILE record
	WORD		NextAttrId;		// 0x28	Next Attribute Id
	WORD		Align;			// 0x2A	Align to 4 byte boundary
	DWORD		RecordNo;		// 0x2C Number of this MFT Record
} FILE_RECORD_HEADER;

// Attribute Header

#define	ATTR_TYPE_STANDARD_INFORMATION	0x10
#define	ATTR_TYPE_ATTRIBUTE_LIST		0x20
#define	ATTR_TYPE_FILE_NAME				0x30
#define	ATTR_TYPE_OBJECT_ID				0x40
#define	ATTR_TYPE_SECURITY_DESCRIPTOR	0x50
#define	ATTR_TYPE_VOLUME_NAME			0x60
#define	ATTR_TYPE_VOLUME_INFORMATION	0x70
#define	ATTR_TYPE_DATA					0x80
#define	ATTR_TYPE_INDEX_ROOT			0x90
#define	ATTR_TYPE_INDEX_ALLOCATION		0xA0
#define	ATTR_TYPE_BITMAP				0xB0
#define	ATTR_TYPE_REPARSE_POINT			0xC0
#define	ATTR_TYPE_EA_INFORMATION		0xD0
#define	ATTR_TYPE_EA					0xE0
#define	ATTR_TYPE_LOGGED_UTILITY_STREAM	0x100

#define	ATTR_FLAG_COMPRESSED			0x0001
#define	ATTR_FLAG_ENCRYPTED				0x4000
#define	ATTR_FLAG_SPARSE				0x8000

typedef	struct tagATTR_HEADER_COMMON
{
	DWORD		Type;			// 0x00	Attribute Type
	DWORD		TotalSize;		// 0x04	Length (including this header)
	BYTE		NonResident;	// 0x08	0 - resident, 1 - non resident
	BYTE		NameLength;		// 0x09	name length in words
	WORD		NameOffset;		// 0x0A	offset to the name
	WORD		Flags;			// 0x0C	Flags
	WORD		Id;				// 0x0E	Attribute Id
} ATTR_HEADER_COMMON;

typedef	struct tagATTR_HEADER_RESIDENT
{
	ATTR_HEADER_COMMON	Header;			// 0x00	Common data structure
	DWORD				AttrSize;		// 0x10	Length of the attribute body
	WORD				AttrOffset;		// 0x14	Offset to the Attribute
	BYTE				IndexedFlag;	// 0x16	Indexed flag
	BYTE				Padding;		// 0x17	Padding
} ATTR_HEADER_RESIDENT;

typedef struct tagATTR_HEADER_NON_RESIDENT
{
	ATTR_HEADER_COMMON	Header;			// 0x00	Common data structure
	ULONGLONG			StartVCN;		// 0x10	Starting VCN
	ULONGLONG			LastVCN;		// 0x18	Last VCN
	WORD				DataRunOffset;	// 0x20	Offset to the Data Runs
	WORD				CompUnitSize;	// 0x22	Compression unit size
	DWORD				Padding;		// 0x24	Padding
	ULONGLONG			AllocSize;		// 0x28	Allocated size of the attribute
	ULONGLONG			RealSize;		// 0x30	Real size of the attribute
	ULONGLONG			IniSize;		// 0x38	Initialized data size of the stream 
} ATTR_HEADER_NON_RESIDENT;


// Attribute: STANDARD_INFORMATION

#define	ATTR_STDINFO_PERMISSION_READONLY	0x00000001
#define	ATTR_STDINFO_PERMISSION_HIDDEN		0x00000002
#define	ATTR_STDINFO_PERMISSION_SYSTEM		0x00000004
#define	ATTR_STDINFO_PERMISSION_ARCHIVE		0x00000020
#define	ATTR_STDINFO_PERMISSION_DEVICE		0x00000040
#define	ATTR_STDINFO_PERMISSION_NORMAL		0x00000080
#define	ATTR_STDINFO_PERMISSION_TEMP		0x00000100
#define	ATTR_STDINFO_PERMISSION_SPARSE		0x00000200
#define	ATTR_STDINFO_PERMISSION_REPARSE		0x00000400
#define	ATTR_STDINFO_PERMISSION_COMPRESSED	0x00000800
#define	ATTR_STDINFO_PERMISSION_OFFLINE		0x00001000
#define	ATTR_STDINFO_PERMISSION_NCI			0x00002000
#define	ATTR_STDINFO_PERMISSION_ENCRYPTED	0x00004000

typedef struct tagATTR_STANDARD_INFORMATION
{
	ULONGLONG	CreateTime;		// 0x18	File creation time
	ULONGLONG	AlterTime;		// 0x20	File altered time
	ULONGLONG	MFTTime;		// 0x28	MFT changed time
	ULONGLONG	ReadTime;		// 0x30	File read time
	DWORD		Permission;		// 0x38	Dos file permission
	DWORD		MaxVersionNo;	// 0x3C	Maxim number of file versions
	DWORD		VersionNo;		// 0x40	File version number
	DWORD		ClassId;		// 0x44	Class Id
	DWORD		OwnerId;		// 0x48	Owner Id
	DWORD		SecurityId;		// 0x4C	Security Id
	ULONGLONG	QuotaCharged;	// 0x54	Quota charged
	ULONGLONG	USN;			// 0x5C	USN Journel
} ATTR_STANDARD_INFORMATION;


// Attribute: ATTRIBUTE_LIST

typedef struct tagATTR_ATTRIBUTE_LIST
{
	DWORD		AttrType;		// Attribute type
	WORD		RecordSize;		// Record length
	BYTE		NameLength;		// Name length in characters
	BYTE		NameOffset;		// Name offset
	ULONGLONG	StartVCN;		// Start VCN
	ULONGLONG	BaseRef;		// Base file reference to the attribute
	WORD		AttrId;			// Attribute Id
} ATTR_ATTRIBUTE_LIST;

// Attribute: FILE_NAME

#define	ATTR_FILENAME_FLAG_READONLY		0x00000001
#define	ATTR_FILENAME_FLAG_HIDDEN		0x00000002
#define	ATTR_FILENAME_FLAG_SYSTEM		0x00000004
#define	ATTR_FILENAME_FLAG_ARCHIVE		0x00000020
#define	ATTR_FILENAME_FLAG_DEVICE		0x00000040
#define	ATTR_FILENAME_FLAG_NORMAL		0x00000080
#define	ATTR_FILENAME_FLAG_TEMP			0x00000100
#define	ATTR_FILENAME_FLAG_SPARSE		0x00000200
#define	ATTR_FILENAME_FLAG_REPARSE		0x00000400
#define	ATTR_FILENAME_FLAG_COMPRESSED	0x00000800
#define	ATTR_FILENAME_FLAG_OFFLINE		0x00001000
#define	ATTR_FILENAME_FLAG_NCI			0x00002000
#define	ATTR_FILENAME_FLAG_ENCRYPTED	0x00004000
#define	ATTR_FILENAME_FLAG_DIRECTORY	0x10000000
#define	ATTR_FILENAME_FLAG_INDEXVIEW	0x20000000

#define	ATTR_FILENAME_NAMESPACE_POSIX	0x00
#define	ATTR_FILENAME_NAMESPACE_WIN32	0x01
#define	ATTR_FILENAME_NAMESPACE_DOS		0x02

typedef struct tagATTR_FILE_NAME
{
	ULONGLONG	ParentRef;		// File reference to the parent directory
	ULONGLONG	CreateTime;		// File creation time
	ULONGLONG	AlterTime;		// File altered time
	ULONGLONG	MFTTime;		// MFT changed time
	ULONGLONG	ReadTime;		// File read time
	ULONGLONG	AllocSize;		// Allocated size of the file
	ULONGLONG	RealSize;		// Real size of the file
	DWORD		Flags;			// Flags
	DWORD		ER;				// Used by EAs and Reparse
	BYTE		NameLength;		// Filename length in characters
	BYTE		NameSpace;		// Filename space
	WORD		Name[1];		// Filename
} ATTR_FILE_NAME;



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
    SetFilePointer(device, offset, &h_off, FILE_BEGIN); // Set a Point to Read

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
ULONGLONG readToNumber(BYTE* buffer, int offset, int len) {
    ULONGLONG result = 0;
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

/// @brief print byte array as ASCII chain
/// @param str array to print
/// @param len number of bytes to print
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

/// @brief print byte array as 02Hex chain
/// @param str array to print
/// @param len number of bytes to print
void printByteHex(BYTE * buffer, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02X ", buffer[i]);
        if (i%16 == 15) {
            printf("\n");
        }
    }
    printf("\n");
}

// Elements include files and folders
typedef struct tagElement {
	wstring name;
	ULONGLONG size;
	char type;
	DWORD ID;
	DWORD parentID;
	ULONGLONG dataOffset;
} Element;

// text color
enum Color { LightBlue = 9, Yellow = 6, Green = 2, White = 7};

// compare elements by their name
bool compareName(Element a, Element b) {
	return a.name < b.name;
}

/// @brief show header for name, size, type and $MFT Number
/// @param color header color
void showHeader(int color) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, color);
	wcout << setw(50) << left << L"Name";
	wcout << setw(15) << left << L"Size";
	wcout << setw(5) << left << L"Type";
	wcout << setw(15) << right << L"MFT Entry" << endl;
	// Show line space
	SetConsoleTextAttribute(handle, White);
	wcout << setfill(L'-');
	wcout << setw(85) << L"-" << endl;
	wcout << setfill(L' ');
}

/// @brief print the element information
/// @param e the element
/// @param color text color
void showContent(Element e, int color) {
	// Set text color
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, color);
	wcout << setw(50) << left << e.name;
	wcout << setw(15) << left << e.size;
	wcout << setw(5) << left << e.type;
	wcout << setw(15) << right << e.ID << endl;
	// Reset text color
	SetConsoleTextAttribute(handle, White);
}

/// @brief show header and all of elements
/// @param Elements vector of elements
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

#endif