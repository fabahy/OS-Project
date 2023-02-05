/*
* This class is used to read FAT32.
*/

#include "util.h"
#define SECTOR_SIZE 512


BootSector::BootSector() {}

BootSector::BootSector(string path)
{
    this->load(path);
}

BootSector::~BootSector() {}

/**
 * Read specific boot sector.
 *
 * @param[in] path disk directory
 * 
 * @return 0 in case of success,
 * @return 1 if can not open disk, 
 * @return 2 if the specific sector is not boot sector,
 * @return 3 otherwise
 */
int BootSector::load(string path)
{
    BYTE* buffer = new BYTE[SECTOR_SIZE];
    this->sPath = path;
    if (LoadDisk(path.c_str(), 0, buffer, SECTOR_SIZE) != 0) {
        wcout << L"ERROR loading disk\n";
        return 1;
    };
    memcpy(BS_jmpBoot, buffer, SECTOR_SIZE);
    readToArr(BS_jmpBoot, buffer, 0, 3);
    readToArr(BS_OEMName, buffer, 3, 8);
    BPB_BytsPerSec = readToNumber(buffer, 11, 2);
    BPB_SecPerClus = buffer[13];
    BPB_RsvdSecCnt = readToNumber(buffer, 14, 2);
    BPB_NumFATs = buffer[16];
    BPB_RootEntCnt = readToNumber(buffer, 17, 2);
    BPB_TotSec16 = readToNumber(buffer, 19, 2);
    BPB_Media = buffer[21];
    BPB_FATSz16 = readToNumber(buffer, 22, 2);
    BPB_SecPerTrk = readToNumber(buffer, 24, 2);
    BPB_NumHeads = readToNumber(buffer, 26, 2);
    BPB_HiddSec = readToNumber(buffer, 28, 4);
    BPB_TotSec32 = readToNumber(buffer, 32, 4);
    BPB_FATSz32 = readToNumber(buffer, 36, 4);
    BPB_ExFlags = readToNumber(buffer, 40, 2);
    BPB_FS_Ver = readToNumber(buffer, 42, 2);
    BPB_RootClus = readToNumber(buffer, 44, 4);
    BPB_FSInfo = readToNumber(buffer, 48, 2);
    BPB_BkBootSec = readToNumber(buffer, 50, 2);
    readToArr(BPB_Reserved, buffer, 52, 12);
    BS_DrvNum = buffer[64];
    BS_Reserved1 = buffer[65];
    BS_BootSig = buffer[66];
    BS_VolID = readToNumber(buffer, 67, 4);
    readToArr(BS_VolLab, buffer, 71, 11);
    readToArr(BS_FilSysType, buffer, 82, 8);
    readToArr(BootCode, buffer, 90, 420);
    readToArr(EndCode, buffer, 510, 2);
    delete[] buffer;

    const char* BS_SIG = "FAT32   ";
    if (strncmp((const char*)BS_FilSysType, BS_SIG, 8) && this->EndCode[0] == 0x55 && EndCode[1] == 0xAA) {
        return 0;
    }
    else {
        return 2;
    }
    return 3;
}

void BootSector::print() {
    _setmode(_fileno(stdout), _O_TEXT);
    cout << setw(30) << left << "Jump Boot Code: "; printByteHex(BS_jmpBoot, 3);
    cout << setw(30) << left << "Version/OS: "; printByteASCII(BS_OEMName, 8);
    cout << setw(30) << left << "Bytes per Sector: " << (int)BPB_BytsPerSec << endl;
    cout << setw(30) << left << "Sectors per Cluster: " << (int)BPB_SecPerClus << endl;
    cout << setw(30) << left << "Reserved Sector Count: " << (int)BPB_RsvdSecCnt << endl; 
    cout << setw(30) << left << "Number FATs: " << (int)BPB_NumFATs << endl;
    cout << setw(30) << left << "Root Entry Count: " << (int)BPB_RootEntCnt << endl;
    cout << setw(30) << left << "Total Sector 16: " << (int)BPB_TotSec16 << endl;
    cout << setw(30) << left << "Media: " << (int)BPB_Media << endl;
    cout << setw(30) << left << "FAT16 size: " << (int)BPB_FATSz16 << endl;
    cout << setw(30) << left << "Sectors per Track: " << (int)BPB_SecPerTrk << endl;
    cout << setw(30) << left << "Number Heads: " << (int)BPB_NumHeads << endl;
    cout << setw(30) << left << "Hidden Sector: " << (int)BPB_HiddSec << endl;
    cout << setw(30) << left << "Total Sector 32: " << (int)BPB_TotSec32 << endl;
    cout << setw(30) << left << "FAT Size 32: " << (int)BPB_FATSz32 << endl;
    cout << setw(30) << left << "Extra Flags 32: " << (int)BPB_ExFlags << endl;
    cout << setw(30) << left << "FAT32 Version: " << (int)BPB_FS_Ver << endl;
    cout << setw(30) << left << "Root Cluster: " << (int)BPB_RootClus << endl;
    cout << setw(30) << left << "FS Info: " << (int)BPB_FSInfo << endl;
    cout << setw(30) << left << "BK Boot Sector: " << (int)BPB_BkBootSec << endl; 
    cout << setw(30) << left << "Reserved: "; printByteHex(BPB_Reserved, 12);
    cout << setw(30) << left << "Driver Number: " << (int)BS_DrvNum << endl;
    cout << setw(30) << left << "Reserved 1: " << (int)BS_Reserved1 << endl;
    cout << setw(30) << left << "Boot Signature: " << (int)BS_BootSig << endl;
    cout << setw(30) << left << "Volume ID: " << (int)BS_VolID << endl;
    _setmode(_fileno(stdout), _O_WTEXT);
}

long long BootSector::FirstDataSector() {
    return this->BPB_RsvdSecCnt + (this->BPB_NumFATs * this->BPB_FATSz32);
}

long long BootSector::FirstFAT1Sector() {
    return this->BPB_RsvdSecCnt;
}

long long BootSector::FirstFAT2Sector() {
    if (this->BPB_NumFATs != 2) {
        return 0;
    }
    else {
        return this->BPB_RsvdSecCnt + this->BPB_FATSz32;
    }
}

string BootSector::getPath() {
    return this->sPath;
}