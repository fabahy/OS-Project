#include "util.h"

FAT32::FAT32() {
    this->EntryPerSector = 128;
    this->BytePerSector = 512;
    this->SectorPerCluster = 0;
    this->FirstDataSector = 0;
    this->sPath = "";
    this->FirstFAT1Sector = 0;
    this->FirstFAT2Sector = 0;
    this->cache = NULL;
    this->sector_cache = 0;
}

FAT32::FAT32(BootSector* BS) {
    this->EntryPerSector = 128;
    this->BytePerSector = 512;
    this->SectorPerCluster = 0;
    this->FirstDataSector = 0;
    this->sPath = "";
    this->FirstFAT1Sector = 0;
    this->FirstFAT2Sector = 0;
    this->cache = NULL;
    this->sector_cache = 0;

    this->load(BS);
}

FAT32::~FAT32() {
    if (this->cache) {
        delete[] this->cache;
    }
}

void FAT32::loadCache(long long newSectorCache) {
    if (this->sector_cache != newSectorCache) {
        this->sector_cache = newSectorCache;
        LoadDisk(this->sPath.c_str(), this->sector_cache*this->BytePerSector, this->cache, this->BytePerSector);
    }
}

void FAT32::load(BootSector* BS) {
    this->EntryPerSector = BS->BPB_BytsPerSec/4;
    this->BytePerSector = BS->BPB_BytsPerSec;
    this->SectorPerCluster = BS->BPB_SecPerClus;
    this->FirstDataSector = BS->FirstDataSector();
    this->sPath = BS->getPath();
    this->FirstFAT1Sector = BS->FirstFAT1Sector();
    this->FirstFAT2Sector = BS->FirstFAT2Sector();
    if (this->cache) {
        delete[] this->cache;
    }
    this->cache = new BYTE[this->BytePerSector];
    this->sector_cache = this->FirstFAT1Sector;
    this->loadCache(this->FirstFAT1Sector);
}

void FAT32::printCache() {
    printByteHex(this->cache, this->BytePerSector);
}

DWORD FAT32::nextClusterOf(DWORD curCluster) {
    long long sectorOffset = curCluster / this->EntryPerSector;
    long long curSector = sectorOffset + this->FirstFAT1Sector;
    this->loadCache(curSector);
    long long entryOffset = (curCluster % this->EntryPerSector) * 4;
    DWORD nextCluster = readToNumber(this->cache, entryOffset, 4);
    return nextCluster;
}
