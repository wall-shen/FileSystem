#include "FileArchive.h"
#include "FileLoader.h"
#include <string.h>

FWriteArchive::FWriteArchive(const char* InFileName, int64 inPos, int64 InBufferSize)
    : fileName(InFileName)
    , pos(inPos)
    , bufferSize(InBufferSize)
    , bufferCount(0)
{  
#ifdef WINDOWS
    handle = FWindowsLoader::GetFWindwsLoader() -> OpenWrite(InFileName, false);
#endif

#ifdef LINUX
    handle = FLinuxLoader::GetFLinuxLoader() -> OpenWrite(InFileName, false);
#endif
    isWrite = true;
    isRead = false;
    buffer = new uint8[InBufferSize];
}


FWriteArchive::FWriteArchive(FHandle* inHandle, const char* InFileName, int64 inPos, int64 InBufferSize)
    : handle(inHandle)
    , fileName(InFileName)
    , pos(inPos)
    , bufferSize(InBufferSize)
    , bufferCount(0)
{
    isWrite = true;
    isRead = false;
    buffer = new uint8[InBufferSize];
}

bool FWriteArchive::Seek(int64 inPos){
    if(!FlushBuffer())
        return false;
    if(!SeekHandle(inPos))
        return false;
    pos = inPos;
    return true;
}

bool FWriteArchive::Close(){
    Flush();
    if(!FlushBuffer())
        return false;
    if(!CloseHandle())
        return false;
    return true;
}

bool FWriteArchive::FlushBuffer(){
    if(bufferCount <= 0 || bufferCount > bufferSize)
        return true;
    if(!WriteToHandle(buffer, bufferCount))
        return false;
    bufferCount = 0;
    return true;
}

bool FWriteArchive::Flush(){
    if(!(FlushBuffer() && handle != nullptr))
        return false;
    if(!(handle -> Flush()))
        return false;
    return true;
}

int64 FWriteArchive::Size(){
    if(handle)
        return handle -> Size();
    else
        return -1;
}

bool FWriteArchive::SeekHandle(int64 inPos){
    if(!handle)
        return false;
    if(!(handle -> Seek(inPos)))
        return false;
    return true;
}
    
bool FWriteArchive::CloseHandle(){
    if(!handle)
        return false;
    if(!(handle -> Close()))
        return false;
    return true;
}

bool FWriteArchive::WriteToHandle(const uint8* pValue, int64 length){
    if(!handle || !pValue || length <= 0)
        return false;
    int64 copySize = handle -> Write(pValue, length);
    if(copySize != length)
        return false;
    return true;
}

void FWriteArchive::Serialize(void* pValue, int64 length){
    // pos += length;
    // if(!pValue || length <= 0)
    //     return;
    // if(length >= bufferSize){
    //     FlushBuffer();
    //     if(!WriteToHandle((uint8*)pValue, length))
    //         DEBUG("Write to Handle failed");
    // }
    // else{
    //     int64 copySize;
    //     while(length > (copySize = bufferSize - bufferCount)){
    //         memcpy(buffer + bufferCount, pValue, copySize);
    //         bufferCount += copySize;
    //         pValue = (uint8*)pValue + copySize;
    //         length -= copySize;
    //         FlushBuffer();
    //     }
    //     if(length > 0){
    //         memcpy(buffer + bufferCount, pValue, length);
    //         bufferCount += length;
    //     }
    // }
    if(length > 0){
        if(!WriteToHandle((uint8*)pValue, length))
            DEBUG("Write to Handle failed");
    }
}

FWriteArchive::~FWriteArchive(){
    if(!Close())
        DEBUG("FWriteArchive close the handle failed");
    if(buffer)
        delete[] buffer;
    if(handle)
        delete handle;
}



FReadArchive::FReadArchive(const char* inFileName, int64 inBufferSize)
    : fileName(inFileName)
    , bufferSize(inBufferSize)
    , bufferPos(0)
    , pos(0)
    , bufferCount(0)
{
#ifdef WINDOWS
    handle = FWindowsLoader::GetFWindwsLoader() -> OpenRead(inFileName, false);
#endif

#ifdef LINUX
    handle = FLinuxLoader::GetFLinuxLoader() -> OpenRead(inFileName);
#endif
    size = handle -> Size();
    isRead = true;
    isWrite = false;
    buffer = new uint8[bufferSize];   
}

FReadArchive::FReadArchive(FHandle* inHandle, const char* inFileName, int64 inSize, int64 inBufferSize)
    : handle(inHandle)
    , fileName(inFileName)
    , size(inSize)
    , bufferSize(inBufferSize)
    , bufferPos(0)
    , pos(0)
    , bufferCount(0)
{
    isRead = true;
    isWrite = false;
    buffer = new uint8[bufferSize];   
}

bool FReadArchive::Seek(int64 inPos){
    if(inPos < 0 || inPos > size)
        return false;
    if(!SeekHandle(inPos))
        return false;
    pos = inPos;
    if(pos >= bufferPos && pos <= bufferPos + bufferCount - 1){
        return true;
    }
    else{
        bufferPos = pos;
        bufferCount = 0;
        return true;
    }
}

bool FReadArchive::Close(){
    if(!CloseHandle())
        return false;
    return 
        true;
}

template<class CompType>
CompType Min(CompType comp1, CompType comp2){
    return comp1 > comp2 ? comp2 : comp1;
}

bool FReadArchive::PreCache(int64 length){
    if(length <= 0)
        return false;
    if(!bufferPos || !bufferCount || bufferPos != pos){
        bufferPos = pos;
        bufferCount = Min(Min(length, bufferSize), size - pos);
        if(bufferCount <= 0)
            return true;
        bool result = ReadFromHandle(buffer, bufferCount);
        if(!result)
            return false;
    }
    return true;
}

void FReadArchive::Serialize(void* pValue, int64 length){    
    if(!pValue || length <= 0)
        return;
    if(pos + length > size){
        DEBUG("FReadArchive::Serialize out of range of file!");
        return;
    }
    // while(length > 0){
    //     int64 copySize = pos >=  bufferPos ? Min(length, bufferPos + bufferCount - pos) : -1;
    //     if(copySize <= 0){
    //         if(length > bufferSize){
    //             bool result = ReadFromHandle((uint8*)pValue, length);
    //             if(!result){
    //                 DEBUG("FReadArchive::Serialize ReadFromHanle failed!");
    //                 return;
    //             }
    //             pos += length;
    //             return;
    //         }
    //         if(!PreCache(bufferSize - 1)){
    //             DEBUG("FReadArchive::Serialize PreCache failed!");
    //             return;
    //         }
    //         copySize = Min(length, bufferPos + bufferSize - pos);
    //         if( copySize <= 0){
    //             DEBUG("FReadArchive::Serialize there is something worong!");
    //         }
    //     }
    //     memcpy(pValue, buffer, copySize);
    //     pValue = (uint8*)pValue + copySize;
    //     length -= copySize;
    //     pos += copySize;
    // }

    bool result = ReadFromHandle((uint8*)pValue, length);
    if(!result){
        DEBUG("FReadArchive::Serialize ReadFromHanle failed!");
        return;
    }
}

bool FReadArchive::SeekHandle(int64 inPos){
    if(!handle)
        return false;
    if(!(handle -> Seek(inPos)))
        return false;
    return true;
}

bool FReadArchive::CloseHandle(){
    if(!handle)
        return true;
    if(!(handle -> Close()))
        return false;
    return true;
}

bool FReadArchive::ReadFromHandle(uint8* pValue, int64 length){
    if(!pValue || length <= 0 || !handle)
        return false;
    int64 copySize = handle -> Read(pValue, length);
    if(copySize != length)
        return false;
    return true;
}

FReadArchive::~FReadArchive(){
    if(!Close())
        DEBUG("FReadArchive close the handle failed! ");
    if(buffer)
        delete[] buffer;
    if(handle)
        delete handle;
}