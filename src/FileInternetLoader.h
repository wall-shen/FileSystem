#ifndef FILEINTERNETLOADERH
#define FILEINTERNETLOADERH

#include "FileLoader.h"
#include "FileArchive.h"
#include "cJSON.h"
#include <curl/curl.h>

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

    Resume(const char* InFileName, uint64 InSize, uint32 InVersion)
        : fileName(InFileName)
        , size(InSize)
        , version(InVersion)
        , pos(0)
    {
#ifdef LINUX
        physicalLoader = FLinuxLoader::GetFLinuxLoader();
#endif

#ifdef WINDOWS
        physicalLoader = FWindowsLoader::GetFWindowsLoader();
#endif
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
    uint8* GetData(){ return data;}

    ~Resume();
};


class FInternetHandle : public FHandle{
    const char* ip;
    CURL* curl;
    const char* fileName;
public:
    FInternetHandle(const char* InIp, const char* InFileName, int64 InPos, int64 InSize);
    virtual int64 Read(uint8* inBuffer, int64 bytesToRead);
    virtual int64 Write(const uint8* outBrffer, int64 bytesToWrite);
    virtual bool Seek(int64 newPosition);
    virtual bool SeekFromEnd(int64 newPosition);
    virtual bool Flush();
    virtual bool Close();
    virtual ~FInternetHandle();
};

class FInternetLoader : public FLoader{
private:
    const char* ip;
public:
    FInternetLoader(const char* inIp = INTERNET_IP)
        : ip(inIp)
    {}
        
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


#endif