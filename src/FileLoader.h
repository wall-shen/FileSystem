#ifndef FileLoaderH
#define FileLoaderH

#include "FileLoaderTypes.h"
#include "FileString.h"
#include "FileArray.h"
#include <iostream>
#include <io.h>

#define LINUX

#ifndef DEBUG
#define DEBUG
#endif

#ifdef DEBUG
#define DEBUG(a) std::cout << a << std::endl
#else
#define DEBUG(a)
#endif

class FHandle{
protected:
    int64 pos;
    int64 size;
public:
    FHandle(int64 InPos, int64 InSize)
        : pos(InPos)
        , size(InSize)
    {       
    }
    /* 
     * Read bytes to Buffer
     * @param inBuffer      Buffer to read
     * @param BytesToRead   Number of bytes to read
     * @return              the bytes have to be readed successfully
     */
    virtual int64 Read(uint8* inBuffer, int64 bytesToRead) = 0;

    /* 
     * Write bytes to Buffer
     * @param inBuffer      Buffer to write
     * @param BytesToWrite  Number of bytes to write
     * @return              the bytes have to be writed successfully
     */
    virtual int64 Write(const uint8* outBrffer, int64 bytesToWrite) = 0;
    
    virtual bool Seek(int64 newPosition) = 0;

    /*
     * Set the new position relative to the end of file
     * @param NewPosition   position relative to the end of file, should to be < 0
     * @return              If operation completed successfully
     */
    virtual bool SeekFromEnd(int64 newPosition) = 0;

    virtual int64 Tell(){ return pos; };
    
    // show the size of file
    virtual int64 Size(){ return size; };

    // flush buffer to disk
    virtual bool Flush() = 0;

    virtual bool Close() = 0;
    
    virtual ~FHandle(){}
};

class FWindowsHandle : public FHandle{
    int32 handle;
public:
    FWindowsHandle(int32 Inhandle, int64 InPos, int64 InSize);
    virtual int64 Read(uint8* inBuffer, int64 bytesToRead);
    virtual int64 Write(const uint8* outBrffer, int64 bytesToWrite);
    virtual bool Seek(int64 newPosition);
    virtual bool SeekFromEnd(int64 newPosition);
    virtual bool Flush();
    virtual bool Close();
    virtual ~FWindowsHandle();
};

class FLinuxHandle : public FHandle{
    int32 handle;
public:
    FLinuxHandle(int32 Inhandle, int64 InPos, int64 InSize);
    virtual int64 Read(uint8* inBuffer, int64 bytesToRead);
    virtual int64 Write(const uint8* outBrffer, int64 bytesToWrite);
    virtual bool Seek(int64 newPosition);
    virtual bool SeekFromEnd(int64 newPosition);
    virtual bool Flush();
    virtual bool Close();
    virtual ~FLinuxHandle();
};

class FPhysicalLoader;

class FLoader{
protected:
    FPhysicalLoader* physicalLoader;
    FLoader* lowerFloader;
public:
    FLoader(){}
    // Get fileLoad lower than this file
    virtual FLoader* GetLowerLoader() = 0;

    // Set fileLoad lower than this file
    virtual bool SetLowerLoader(FLoader* wrappedLoader) = 0;
    
    virtual int64 FileSize(const char* fileName) = 0;

    // If the file is Existing
    virtual bool FileExists(const char* fileName) = 0;

    virtual bool FileDelete(const char* fileName) = 0;

    virtual bool FileMove(const char* fileFrom, const char* fileTo) = 0;

    virtual bool FileCopy(const char* fileFrom, const char* fileTo) = 0;

    /*
     * Return read handle of file
     * @param fileName  the name of file to read
     * @return          the read handle of file
     */
    virtual FHandle* OpenRead(const char* fileName) = 0;

    /*
     * Return write handle of file
     * @param fileName  the name of file to write
     * @param append    if append to the end of file
     * @return          the write handle of file
     */
    virtual FHandle* OpenWrite(const char* fileName, bool append) = 0;

    // Directory operation
    virtual bool DirectoryExist(const char* directoryName) = 0;

    virtual bool DirectoryCreate(const char* directoryName) = 0;

    virtual bool DirectoryDelete(const char* directoryName) = 0;
    
    /*
     * Find files with extension in the given directory
     * @param foundFiles    array to save the files found
     * @param directory     path to find files
     * @param extension     file filter
     */
    virtual void FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension) = 0;

     /*
     * Find files with extension in the given directory recursively
     * @param foundFiles    array to save the files found
     * @param directory     path to find files
     * @param extension     file filter
     */
    virtual void FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension) = 0;

    virtual ~FLoader(){}

};

class FPhysicalLoader : public FLoader{
public:

    // physicalLoader has no lowerLoader
    virtual FLoader* GetLowerLoader() final { return nullptr; }

    virtual bool SetLowerLoader(FLoader* wrappedLoader) final { return false; }

    virtual ~FPhysicalLoader(){}
};

class FWindowsLoader : public FPhysicalLoader{
private:
    FWindowsLoader(){}
    static FWindowsLoader* singleFWindowsLoader;
public:
    static FWindowsLoader* GetFWindowsLoader();

    virtual int64 FileSize(const char* fileName);

    virtual int64 FileSize(int32 handle);

    virtual bool FileExists(const char* fileName);

    virtual bool FileDelete(const char* fileName);

    virtual bool FileMove(const char* fileFrom, const char* fileTo);

    virtual bool FileCopy(const char* fileFrom, const char* fileTo);

    virtual FHandle* OpenRead(const char* fileName);

    virtual FHandle* OpenWrite(const char* fileName, bool append);

    virtual bool DirectoryExist(const char* directoryName);

    virtual bool DirectoryCreate(const char* directoryName);

    virtual bool DirectoryDelete(const char* directoryName);

    virtual void FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension);

    virtual void FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension);

    virtual ~FWindowsLoader(){}
};


class FLinuxLoader : public FPhysicalLoader{
private:
    FLinuxLoader(){}
    static FLinuxLoader* singleFLinuxLoader;
public:
    static FLinuxLoader* GetFWindowsLoader();

    virtual int64 FileSize(const char* fileName);

    virtual bool FileExists(const char* fileName);

    virtual bool FileDelete(const char* fileName);

    virtual bool FileMove(const char* fileFrom, const char* fileTo);

    virtual bool FileCopy(const char* fileFrom, const char* fileTo);

    virtual FHandle* OpenRead(const char* fileName);

    virtual FHandle* OpenWrite(const char* fileName, bool append);

    virtual bool DirectoryExist(const char* directoryName);

    virtual bool DirectoryCreate(const char* directoryName);

    virtual bool DirectoryDelete(const char* directoryName);
    
    virtual void FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension);

    virtual void FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension);

    virtual ~FLinuxLoader(){ }
};
#endif