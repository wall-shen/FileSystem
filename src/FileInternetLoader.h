#ifndef FILEINTERNETLOADERH
#define FILEINTERNETLOADERH

#include "FileLoader.h"
#include "FileArchive.h"

class FInternetLoader : public FLoader{
    FInternetLoader(){}
        
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

    virtual ~FInternetLoader(){}
};


class Resume{
private:
    FPhysicalLoader* physicalLoader;
    uint8* data;
    FString fileName;
    uint64 size;
    uint32 version;
    uint64 pos;
public:
    Resume(FPhysicalLoader* InPhysicalLoader, const char* InFileName, uint64 InSize, uint32 InVersion)
        : fileName(InFileName)
        , physicalLoader(InPhysicalLoader)
        , size(InSize)
        , version(InVersion)
        , pos(0)
    {
        data = new uint8[InSize];
        PreLoad();
    }

    void PreLoad();

    void WriteToDisk();

    int64 Write(uint8* inBuffer, uint64 byteToWrite);

    void Print();

    FString GetFileName(){ return fileName; }    
    uint64 GetSize(){ return size; }
    uint32 GetVsersion(){ return version; }
    uint64 GetPos(){ return pos; }

    ~Resume();
};
#endif