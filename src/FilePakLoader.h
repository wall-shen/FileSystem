#ifndef FILEPAKLOADERH
#define FILEPAKLOADERH

#include "FileLoader.h"
#include "FileArchive.h"
#include <map>

enum{
    NormalFlag   = 0x00,
    DeletedFlag  = 0x01,
    DownloadFlag = 0x02
};

class PakInfo{
public:
    friend class PakFile;
    enum{
        PkgMigic = 0xACD19C0D,
    };
    // offset of index region
    int64 indexOffset;
    // size of index region
    int64 indexSize;
    uint32 magic;
    uint32 version;

public:
    PakInfo(int64 InIndexOffset = 0, int64 InIndexSize = 0, int32 InVersion = 1)
        : indexOffset(InIndexOffset)
        , indexSize(InIndexSize)
        , magic(PkgMigic)
        , version(InVersion)
    {
        indexOffset = sizeof(indexOffset) + sizeof(indexSize) + sizeof(magic) + sizeof(version);
    }

    int64 GetIndexOffset() { return indexOffset; }

    int64 GetIndexSize() { return indexSize; }

    uint64 GetSerializedSize(){
        uint64 size = sizeof(indexOffset) + sizeof(indexSize) + sizeof(magic) + sizeof(version);
        return size;
    }

    void Serialize(FArchive& ar);

    void Print();
};

class FileBlock{
public:
    friend class PakFile;
    // offset of block start 
    uint64 start; 
    // offset of block end
    uint64 end;
    // // point to next Block
    // FileBlock next;
public:
    FileBlock(uint64 InStart = 0, uint64 InEnd = 0)
        : start(InStart)
        , end(InEnd){}
    uint64 GetStart(){ return start; }
    uint64 GetEnd(){ return end; }
    uint64 GetSize(){ return end - start; }
    int64 GetSerializeSize(){ return sizeof(start) + sizeof(end); }
    void Serialize(FArchive& ar){
        ar << start;
        ar << end;
    }
    void Print();
};

typedef FArray<FileBlock> BlockList;
class PakEntry{
public:
    friend class PakFile;
 
    uint32 version;
    uint32 hashOne;
    uint32 hashTwo;
    uint64 compressSize;
    uint64 uncompressSize;
    uint32 compressMethod;
    BlockList blockList;
    uint64 maxBlockSize;
    uint32 fBSize;
    uint8 flag;

public:
    PakEntry() {}
    PakEntry(uint32 inVersion, uint32 InHashOne, uint32 InHashTwo, uint64 IncompressSize, uint64 InUncompressSize, uint32 InCompressMethod, \
        BlockList& InBlockList, uint64 InMaxBlockSize, uint32 InFbSize,uint8 Inflag = NormalFlag)
        : version(inVersion)
        , hashOne(InHashOne)
        , hashTwo(InHashTwo)
        , compressSize(IncompressSize)
        , uncompressSize(InUncompressSize)
        , compressMethod(InCompressMethod)
        , blockList(InBlockList)
        , maxBlockSize(InMaxBlockSize)
        , fBSize(InFbSize)
        , flag(Inflag)
        {}
    int64 GetCompressSize() const { return compressSize; }
    int64 GetUnCompressSize() const { return uncompressSize; }
    int64 GetSerializedSize();
    FArray<FileBlock>& GetFileBlockList() { return blockList; };
    void Serialize(FArchive& ar);
    void SetFlag(uint8 inFlag) { flag = inFlag; };
    int32 getFlag() const { return flag; }
    void Print();
};

/**
 * first                size of block
 * second -> first      offset of block in blockList
 * second -> second     offset of entry in files
 */
typedef std::multimap<uint64, std::pair<int32, int32>> DeleteList;

/**
 *  first               HashOne
 *  second -> first     HashTwo
 *  second -> second    offset of entry in files
 */
typedef std::multimap<uint32, std::pair<uint32, int32>> PakIndex;
class PakFile{
//private::
public:
    FString pakFileName;
    PakInfo info;
    DeleteList deleteList;
    PakIndex index;
    FArray<PakEntry> files;
    int32 entryNum;
    int64 size;
    bool isValid;

public:
    PakFile()
        : size(0)
        , entryNum(0)
        , isValid(false)
    {}
    PakFile(const char* InPakFileName)
        : pakFileName(InPakFileName)
        , entryNum(0)
        , isValid(false)
    {}
    void initialize(FArchive&);
    FString& GetFileName() { return pakFileName; }
    int64 GetSize() { return size; }
    PakIndex& GetIndex() { return index; }
    uint32 GetVersion() { return info.version; }
    void SetVersion(uint32 InVersion) { info.version = InVersion; }
    bool Remove(const char* fileName);
    int64 Read(FHandle* handle, PakEntry&, uint8* inBuffer);
    int64 Write(FHandle* handle, const char* fileName, const uint8* outBrffer, int64 bytesToWrite);
    int64 CreateFile(const char* fileName, int64 size, int64 uncompressSize, uint32 compressMethod);
    void FindFiles(FArray<FString> foundfiles, const char* directory, const char* extension);
    void FindFilesRecursively(FArray<FString> foundfiles, const char* directory, const char* extension);
    bool FindFile(const char* fileName, PakEntry& pakEntry);
    PakInfo& GetInfo(){ return info; }
    int32 GetEntryNum() {return entryNum; }
    void Serialize(FArchive& archive);
    void Print();
    ~PakFile(){}

private:
    bool AddEntryToFiles(const char* fileName, uint64 compressSize, uint64 unCompressSize, uint32 compressMethod, BlockList& blockList);
    void LoadIndex(FArchive&);
    int64 WriteToBlock(FHandle* handle, uint64 startPos, const uint8* outBuffer, int64 byteToWrite);
    bool IsValid() { return isValid; }
    void SetIsValid(bool InFlag) { isValid = InFlag; }
};

class FPakHandle : public FHandle{
private:
    FHandle* physicalHandle;
    int64 pos;
    const PakFile& pakFile;
    PakEntry pakEntry;
    uint32 blockListIndex;
    uint8* data;
public:
    FPakHandle(FHandle* Inhandle, int64 InPos, PakFile& InPakFile, PakEntry& InPakEntry);
    virtual int64 Read(uint8* inBuffer, int64 bytesToRead);
    virtual int64 Write(const uint8* outBrffer, int64 bytesToWrite);
    virtual bool Seek(int64 newPosition);
    virtual bool SeekFromEnd(int64 newPosition);
    virtual bool Flush();
    virtual bool Close();
    virtual ~FPakHandle();
};


class FPakLoader : public FLoader{
// private:
public:
    FArray<PakFile> pakFiles;
    static FPakLoader* singleFPakLoader;
    FPakLoader(const char* defaultDir = PAKLOADER_DIRECTORY);

public:
    static FPakLoader* GetFPakLoader();

    // Get fileLoad lower than this file
    virtual FLoader* GetLowerLoader() { return lowerFloader;}

    // Set fileLoad lower than this file
    virtual bool SetLowerLoader(FLoader* newLowerLoader) { lowerFloader = newLowerLoader;  return true; }
    
    virtual FHandle* GetPhysicalReadHandle(const char* fileName){
        return physicalLoader -> OpenRead(fileName);
    }
    
    virtual FHandle* GetPhysicalWriteHandle(const char* fileName){
        return physicalLoader -> OpenWrite(fileName, false);
    }

    virtual int64 FileSize(const char* fileName);

    // If the file is Existing
    virtual bool FileExists(const char* fileName);

    virtual bool FileDelete(const char* fileName);

    virtual bool FileMove(const char* fileFrom, const char* fileTo);

    virtual bool FileCopy(const char* fileFrom, const char* fileTo);

    /*
     * Return read handle of file
     * @param fileName  the name of file to read
     * @return          the read handle of file
     */
    virtual FHandle* OpenRead(const char* fileName);

    /*
     * Return write handle of file
     * @param fileName  the name of file to write
     * @param append    if append to the end of file
     * @return          the write handle of file
     */
    virtual FHandle* OpenWrite(const char* fileName, bool append = false);

    int64 Write(const char * pakName, const char* fileName, const uint8* outBrffer, int64 bytesToWrite);

    // Directory operation
    virtual bool DirectoryExist(const char* directoryName);

    virtual bool DirectoryCreate(const char* directoryName);

    virtual bool DirectoryDelete(const char* directoryName);
    
    /*
     * Find files with extension in the given directory
     * @param foundFiles    array to save the files found
     * @param directory     path to find files
     * @param extension     file filter
     */
    virtual void FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension);

     /*
     * Find files with extension in the given directory recursively
     * @param foundFiles    array to save the files found
     * @param directory     path to find files
     * @param extension     file filter
     */
    virtual void FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension);

    bool CreatePak(const char* pakName);

    bool CreatFile(const char* pakName);

    virtual ~FPakLoader();
};
#endif