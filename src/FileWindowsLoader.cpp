#include "FileLoader.h"

#ifdef WINDOWS

#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>

enum{
    ReadBufferSize = 10240
};

FWindowsLoader* FWindowsLoader::singleFWindowsLoader = nullptr;

FWindowsLoader* FWindowsLoader::GetFWindowsLoader(){
    if(singleFWindowsLoader == nullptr){
        static FWindowsLoader staticWinsdowsLoader;
        singleFWindowsLoader = &staticWinsdowsLoader;
    }
    return singleFWindowsLoader;
}


FWindowsHandle::FWindowsHandle(int32 Inhandle, int64 InPos, int64 InSize) 
    : FHandle(InPos, InSize)
    , handle(Inhandle)
{}

int64 FWindowsHandle::Read(uint8* inBuffer, int64 bytesToRead){
    int64 readSize = read(handle, inBuffer, bytesToRead);
    pos += readSize;
    return readSize;
}

int64 FWindowsHandle::Write(const uint8* outBuffer, int64 bytesToWrite){
    int64 writeSize = write(handle, outBuffer, bytesToWrite);
    pos += writeSize;
    return writeSize;
}

/*
 * change read/write pointer to new position
 * @return      true successfully, false wrong, when newPosition > file size return true
 */    
bool FWindowsHandle::Seek(int64 newPosition){
    if(lseek(handle, newPosition, SEEK_SET) != -1L){
        pos = lseek(handle, 0, SEEK_CUR);
        return true;
    }
    else
        return false;
}

bool FWindowsHandle::SeekFromEnd(int64 newPosition){
    if(lseek(handle, newPosition, SEEK_END) != -1L){
        pos = lseek(handle, 0, SEEK_CUR);
        return true;
    }
    else
        return false;
}

int64 FWindowsHandle::Tell(){
    return lseek(handle, 0, SEEK_CUR);
}
int64 FWindowsHandle::int64 Size(){
    size = lseek(handle, 0, SEEK_END);
    Seek(pos);
    return size;
}

bool FWindowsHandle::Flush(){
    if(_commit(handle) == 0)
        return true;
    else
        return false;
}

bool FWindowsHandle::Close(){
    if(close(handle) == 0)
        return true;
    else
        return false;
}

FWindowsHandle::~FWindowsHandle(){
        Close();
}


int64 FWindowsLoader::FileSize(const char* fileName){
    int32 handle = open(fileName, O_RDWR | O_BINARY);
    if(handle == -1)
        return -1;
    else
        return filelength(handle); 
}

int64 FWindowsLoader::FileSize(int32 handle){
    return filelength(handle); 
}

bool FWindowsLoader::FileExists(const char* fileName){
    int32 result = access(fileName, 0);
    if(result == 0)
        return true;
    else
        return false;
}

bool FWindowsLoader::FileDelete(const char* fileName){
    int32 result = remove(fileName);
    if(result == 0)
        return true;
    else
        return false;
}

bool FWindowsLoader::FileMove(const char* fileFrom, const char* fileTo){
    if(!FileExists(fileFrom))
        return false;
    int32 inHandle = open(fileFrom, O_RDONLY | O_BINARY);
    int32 outHandle = open(fileTo, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
    uint8 buffer[ReadBufferSize];
    int32 copySize = 0;
    while((copySize = read(inHandle, buffer, ReadBufferSize)) != 0){
        if(copySize == -1){
            close(inHandle);
            close(outHandle);
            return false;
        }
        int getSize = write(outHandle, buffer, copySize);
        if(getSize != copySize){
            close(inHandle);
            close(outHandle);
            return false;
        }
    }
    close(inHandle);
    close(outHandle);
    if(!FileDelete(fileFrom)){
        FileDelete(fileTo);
        return false;
    }
    return true;
}

bool FWindowsLoader::FileCopy(const char* fileFrom, const char* fileTo){
    if(!FileExists(fileFrom))
        return false;
    int32 inHandle = open(fileFrom, O_RDONLY | O_BINARY);
    int32 outHandle = open(fileTo, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
    uint8 buffer[ReadBufferSize];
    int32 copySize = 0;
    while((copySize = read(inHandle, buffer, ReadBufferSize)) != 0){
        if(copySize == -1){
            close(inHandle);
            close(outHandle);
            return false;
        }
        int getSize = write(outHandle, buffer, copySize);
        if(getSize != copySize){
            close(inHandle);
            close(outHandle);
            return false;
        }
    }
    close(inHandle);
    close(outHandle);
    return true;
}

FHandle* FWindowsLoader::OpenRead(const char* fileName){
    if(FileExists(fileName)){
        int32 handle = open(fileName, O_RDONLY | O_BINARY);
        if(handle == -1)
            return nullptr;
        FHandle* winHandle = new FWindowsHandle(handle, 0, FileSize(handle));
        return winHandle;
    }
    return nullptr;
}

FHandle* FWindowsLoader::OpenWrite(const char* fileName, bool append){
    int32 handle;
    if(append)
        handle = open(fileName, O_WRONLY | O_BINARY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
    else
        handle = open(fileName, O_WRONLY | O_BINARY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
    if(handle == -1)
        return nullptr;
    FHandle* winHandle = new FWindowsHandle(handle, 0, FileSize(handle));
    return winHandle;
}

bool FWindowsLoader::DirectoryExist(const char* directoryName){
    int32 result = access(directoryName, 0);
    if(result == 0)
        return true;
    else
        return false;
}

bool FWindowsLoader::DirectoryCreate(const char* directoryName){
    if(DirectoryExist(directoryName))
        return true;
    int result = mkdir(directoryName);
    if(result == 0)
        return true;
    else
        return false;
}

bool FWindowsLoader::DirectoryDelete(const char* directoryName){
    int32 result = remove(directoryName);
    if(result == 0)
        return true;
    else
        return false;
}

void FWindowsLoader::FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension){
    _finddata_t fileData;
    FString directoryToFind(directory);
    directoryToFind = directoryToFind + "\\\\*" + extension;
    int64 findHandle = -1;
    findHandle = _findfirst(directoryToFind.GetStr().c_str(), &fileData);
    if(findHandle == -1)
        return;
    do{
        if(strcmp(fileData.name, ".") == 0 || strcmp(fileData.name , "..") == 0)
            continue;
        foundFiles.PushBack(FString(fileData.name));
    }while(_findnext(findHandle, &fileData) == 0);
    _findclose(findHandle);
}

void FWindowsLoader::FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension){
    _finddata_t fileData;
    FString directoryToFind(directory);
    directoryToFind = directoryToFind + "\\\\*" + extension;
    int64 findHandle = -1;
    findHandle = _findfirst(directoryToFind.GetStr().c_str(), &fileData);
    if(findHandle == -1)
        return;
    do{
        if(strcmp(fileData.name, ".") == 0 || strcmp(fileData.name , "..") == 0)
            continue;
        if(fileData.attrib & _A_SUBDIR){
            FString subDir(directory);
            FindFilesRecursively(foundFiles, (subDir + "\\\\" + fileData.name).GetStr().c_str(), extension);
        }

        foundFiles.PushBack(FString(fileData.name));
    }while(_findnext(findHandle, &fileData) == 0);
    _findclose(findHandle);
}

#endif


























// // class FileArchive{
// //     enum{
// //         MaxBufferSize  = 10240,
// //     };
// //     std::fstream fhandle;
// //     uint8* buffer;
// //     PakFile& pakFile;
// //     PakEntry pakEntry;     
// // public:
// //     FileArchive(PakFile& targetPakfile)
// //         : pakFile(targetPakfile){
// //         buffer = new uint8[MaxBufferSize];
// //     }

// //     bool Seek(uint64 pos){
// //         fhandle.seekp(pos);
// //     }

// //     bool Open();
// //     bool Read(uint64 pos, void* outBuffer, uint64 size);
// //     bool Write(const char* fileName, void* inBuffer, uint64 size);
// // };

// bool FileArchive::Open(){
//      if(!fhandle.is_open())
//             fhandle.open(pakFile.GetFileName(), std::ios::binary);
//         if(!fhandle.is_open())
//             return false;
//         else
//             return true;
// }
// uint64 min(uint64 a, uint64 b){
//     return a < b ?  a : b;
// }

// bool FileArchive::Read(uint64 pos, void* outBuffer, uint64 size){
//     if((pos < 0 || pos > pakEntry.GetCompressSize()) && pos + size <= pakEntry.GetOffset() + pakEntry.GetUnCompressSize())
//         return false;
//     if(!fhandle.is_open()){
//         if(!Open())
//             return false;
//     }
    

//     FileBlockList& fb = pakEntry.GetFileBlockList();
//     int i;
//     for(i{} pos > fb[i].GetSize(); i++)
//         pos -= fb[i].GetSize();

//     pos += fb[i].GetStart();
 
//     while(size > 0){
//         if(!Seek(pos))
//             return false;
//         int64 readSize = min(fb[i].GetSize() - pos, size);
//         fhandle.read((char*)outBuffer, readSize);
//         size -= readSize;
//         if( size > 0)
//             pos = fb[++i].GetStart();
//     }

//     return true;
// }

// bool FileArchive::Write(const char* fileName, void* inBuffer, uint64 size){
//     if(pakFile.FindFile(fileName))
//         pakFile.DeleteFile(fileName);
    
//     PakEntry newPakEntry;
//     PakInfo& pakInfo= pakFile.GetInfo();
//     FileBlock newFb(pakInfo.GetindexOffset(), size);
//     newPakEntry.GetFileBlockList().push_back(newFb);

// }