#ifndef HASHH
#define HASHH

#include "FileLoaderTypes.h"

class Crc{
public:
    static int32 crcValueOne;
    static int32 crcValueTwo;
    static uint32 CRCTablesSB8[8][256];
    static uint32 MemCrc32(const void* data, int32 length, uint32 CRC=0);
};

#endif