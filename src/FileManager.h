#include "FilePakLoader.h"
#include "FileInternetLoader.h"

enum{
    PakLoader        =   0x01,
    InternetLoader   =   0x02,
    WindowsLoader    =   0x03,
    LinuxLoader      =   0x04
};

class FManager{
    FArray<FLoader*> loaderList;

public:
    FManager();

    bool AddLoader(int16 loaderIndex, const char* loaderPara = NULL);

    bool ListClear();

    int64 FileSize(const char* fileName);

    // If the file is Existing
    bool FileExists(const char* fileName);

    bool FileDelete(const char* fileName);

    bool FileMove(const char* fileFrom, const char* fileTo);

    bool FileCopy(const char* fileFrom, const char* fileTo);

    /*
     * Return read handle of file
     * @param fileName  the name of file to read
     * @return          the read handle of file
     */
    FHandle* OpenRead(const char* fileName);

    /*
     * Return write handle of file
     * @param fileName  the name of file to write
     * @param append    if append to the end of file
     * @return          the write handle of file
     */
    FHandle* OpenWrite(const char* fileName, bool append);

    // Directory operation
    bool DirectoryExist(const char* directoryName);

    bool DirectoryCreate(const char* directoryName);

    bool DirectoryDelete(const char* directoryName);
    
    /*
     * Find files with extension in the given directory
     * @param foundFiles    array to save the files found
     * @param directory     path to find files
     * @param extension     file filter
     */
    void FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension);

     /*
     * Find files with extension in the given directory recursively
     * @param foundFiles    array to save the files found
     * @param directory     path to find files
     * @param extension     file filter
     */
    void FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension);

    int64 WriteToPak(FHandle* handle, const char* pakName, const char* fileName, int64 size, int32 version);

    void Update();

    ~FManager();
};