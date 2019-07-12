#ifndef FILEPAKLOADERH
#define FILEPAKLOADERH

#include "FileLoader.h"
#include "FileArchive.h"
#include <map>

class PakInfo{
    enum{
        PkgMigic = 0xACD19C0D,
    };
    // offset of index region
    int64 indexOffset;
    // size of index region
    int64 indexSize;
    uint32 magic;

public:
    PakInfo(int64 InIndexOffset = -1, int64 InIndexSize = 0)
        : indexOffset(InIndexOffset)
        , indexSize(InIndexSize)
        , magic(PkgMigic){
        
    }

    int64 GetIndexOffset() { return indexOffset; }

    int64 GetIndexSize() { return indexSize; }

    uint64 GetSerializedSize(){
        uint64 size = sizeof(indexOffset) + sizeof(indexSize) + sizeof(magic);
        return size;
    }

    void Serialize(FArchive& ar);
};

class FileBlock{
    // offset of block start 
    uint64 start; 
    // offset of block end
    uint64 end;
    // // point to next Block
    // FileBlock next;
public:
    FileBlock(uint64 InStart, uint64 InEnd)
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
};

class PakEntry{
    enum{
        NormalFlag  = 0x00,
        DeletedFlag = 0x01
    };
    uint64 compressSize;
    uint64 uncompressSize;
    uint32 compressMethod;
    uint64 offset;
    FArray<FileBlock> blockList;
    uint32 fBSize;
    uint8 flag;

public:
    PakEntry()
        : uncompressSize(0)
        , compressMethod(0)
        , fBSize(0)
        , flag(NormalFlag){

        }
    int64 GetCompressSize() const { return compressSize; }
    int64 GetUnCompressSize() const { return uncompressSize; }
    int64 GetOffset() const { return offset; }
    int64 GetSerializedSize();
    FArray<FileBlock>& GetFileBlockList() { return blockList; };
    void Serialize(FArchive& ar);
    void SetFlag(uint8 inFlag) { flag = inFlag; };
    int32 getFlag() const { return flag; }
};

typedef std::map<FString, std::map<FString, int64>> PakIndex;
class PakFile{
private:
    FString pakFileName;
    PakInfo info;
    PakIndex index;
    FArray<PakEntry> files;
    int32 entryNum;
    FString mountPoint;
    uint32* fileNameHash;
    uint32* fileNameHashIndex;
    int64 size;
    bool isValid;

public:
    PakFile(){};
    FString& GetFileName() { return pakFileName; }
    int64 GetSize() { return size; }
    PakIndex& GetIndex() { return index; }
    void SetMountPoint(const char* path){ mountPoint = path; }
    FString& GetMountPoint() { return mountPoint; }
    void FindFiles(FArray<FString> foundfiles, const char* directory, const char* extension);
    void FindFilesRecursively(FArray<FString> foundfiles, const char* directory, const char* extension);
    bool FindFile(const char* fileName, PakEntry* pakEntry);
    PakInfo& GetInfo(){ return info; }
    void Serialize(FArchive& archive);
    ~PakFile(){}
};

class FPakHandle : public FHandle{
private:
    FHandle* physicalHandle;
    int64 pos;
    const PakFile& pakFile;
    PakEntry& pakEntry;
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
private:
    FArray<PakFile> pakFiles;
public:
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
    virtual FHandle* OpenWrite(const char* fileName, bool append);

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
};
#endif