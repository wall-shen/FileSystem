#ifndef FileArchiveH
#define FileArchiveH
#include "FileLoader.h"
#include "FileLoaderTypes.h"
#include "FileString.h"

enum{
    DefaultBufferSize = 10240,
};


class FArchive{
public:
    FArchive(){}
    FArchive(const FArchive&){};
    FArchive& operator=(const FArchive&){ return *this; };
    virtual ~FArchive(){};

    friend FArchive& operator<<(FArchive& archive, uint8& value){
        archive.Serialize(&value, sizeof(value));
        return archive;
    }

    friend FArchive& operator<<(FArchive& archive, int8& value){
        archive.Serialize(&value, sizeof(value));
        return archive;
    }

    friend FArchive& operator<<(FArchive& archive, uint16& value){
        archive.Serialize(&value, sizeof(value));
        return archive;
    }

    friend FArchive& operator<<(FArchive& archive, int16& value){
        archive.Serialize(&value, sizeof(value));
        return archive;
    }

    friend FArchive& operator<<(FArchive& archive, uint32& value){
        archive.Serialize(&value, sizeof(value));
        return archive;
    }

    friend FArchive& operator<<(FArchive& archive, int32& value){
        archive.Serialize(&value, sizeof(value));
        return archive;
    }

    friend FArchive& operator<<(FArchive& archive, uint64& value){
        archive.Serialize(&value, sizeof(value));
        return archive;
    }

    friend FArchive& operator<<(FArchive& archive, int64& value){
        archive.Serialize(&value, sizeof(value));
        return archive;
     }

    friend FArchive& operator<<(FArchive& archive, bool& value){
        archive.Serialize(&value, sizeof(value));
        return archive;
    }

    friend FArchive& operator<<(FArchive& archive, FString& value){
        int32 strSize = value.GetSize() + 1;
        archive << strSize;
        if(archive.IsRead()){
            char str[strSize];
            archive.Serialize(str, strSize);
            value = str; 
        }
        if(archive.IsWrite()){
            archive.Serialize(&(value.GetStr()), strSize);
        }
        return archive;
    }

    virtual void Serialize(void* pValue, int64 length) = 0;

    virtual bool IsRead() = 0;

    virtual bool IsWrite() = 0;

    virtual bool Seek(int64 inPos) = 0;

    virtual int64 Tell() = 0;

    virtual bool Close() = 0;

    virtual int64 Size() = 0;

    virtual FString GetFileName() = 0;

    virtual bool SeekHandle(int64 inPos) = 0;

    virtual bool CloseHandle() = 0;

};

class FWriteArchive : public FArchive{
private:
    FString fileName;
    FHandle* handle;
    uint8* buffer;
    int64 bufferSize;
    // position in handle
    int64 pos;
    // size of data in buffer
    int64 bufferCount;
    bool isRead;
    bool isWrite;

public:
    FWriteArchive(FHandle* inHandle, const char* InFileName, int64 inPos, int64 InBufferSize = DefaultBufferSize);


    virtual bool IsRead(){ return isRead; }
    
    virtual bool IsWrite(){ return isWrite; }

    virtual bool Seek(int64 inPos);

    virtual int64 Tell(){ return pos; }

    virtual bool Close();

    virtual void Serialize(void* pValue, int64 length);

    // flush the data in buffer to hanle
    virtual bool FlushBuffer();

    // flush the hanle
    virtual bool Flush();

    virtual int64 Size();

    virtual FString GetFileName(){ return fileName; }

    virtual bool SeekHandle(int64 inPos);

    virtual bool CloseHandle();

    /*
     * write the data in buffer to handle
     * @param pValue    pointer to data area
     * @param length    length of data to write
     * @return          if operation completed successfully
     */
    virtual bool WriteToHandle(const uint8* pValue, int64 length);

    virtual ~FWriteArchive();
};
class FReadArchive : public FArchive{
private:
    FString fileName;
    FHandle* handle;
    int64 size;
    uint8* buffer;
    // buffer[0] mapping to position of file
    int64 bufferPos;
    int64 bufferSize;
    // position in file
    int64 pos;
    // size of data in buffer
    int64 bufferCount;
    bool isRead;
    bool isWrite;
    
public:
    FReadArchive(FHandle* inHandle, const char* inFileName, int64 inSize, int64 inBufferSize = DefaultBufferSize);

    virtual bool IsRead(){ return isRead; }
    
    virtual bool IsWrite(){ return isWrite; }

    virtual bool Seek(int64 inPos);

    virtual int64 Tell(){ return pos; }

    virtual bool Close();

    virtual void Serialize(void* pValue, int64 length);

    virtual int64 Size(){ return size; }

    virtual FString GetFileName(){ return fileName; }

    virtual bool SeekHandle(int64 inPos);

    virtual bool CloseHandle();

    /*
     * cache data previously
     * @length          length to read
     * @return          if operation completed successfully
     */
    virtual bool PreCache(int64 length);
    
    /*
     * read the data in buffer to handle
     * @param pValue    pointer of data area to get data
     * @param length    length to read
     * @return          if operation completed successfully
     */
    virtual bool ReadFromHandle(uint8* pValue, int64 length);

    virtual ~FReadArchive();
};

#endif