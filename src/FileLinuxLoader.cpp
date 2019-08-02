#include "FileLoader.h"

#ifdef LINUX
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

enum{
    ReadBufferSize = 10240
};

FLinuxHandle::FLinuxHandle(int32 Inhandle, int64 InPos, int64 InSize)
    : FHandle(InPos, InSize)
    , handle(Inhandle)
{}

int64 FLinuxHandle::Read(uint8* inBuffer, int64 bytesToRead){
    int64 readSize = read(handle, inBuffer, bytesToRead);
    pos += readSize;
    return readSize;
}

int64 FLinuxHandle::Write(const uint8* outBuffer, int64 bytesToWrite){
    int64 writeSize = write(handle, outBuffer, bytesToWrite);
    pos += writeSize;
    return writeSize;
}
bool FLinuxHandle::Seek(int64 newPosition){
    if(lseek(handle, newPosition, SEEK_SET) != -1L){
        pos = lseek(handle, 0, SEEK_CUR);
        return true;
    }
    else
        return false;
}
bool FLinuxHandle::SeekFromEnd(int64 newPosition){
    if(lseek(handle, newPosition, SEEK_END) != -1L){
        pos = lseek(handle, 0, SEEK_CUR);
        return true;
    }
    else
        return false;
}
int64 FLinuxHandle::Tell(){
    return lseek(handle, 0, SEEK_CUR);
}
int64 FLinuxHandle::Size(){
    size = lseek(handle, 0, SEEK_END);
    Seek(pos);
    return size;
}

bool FLinuxHandle::Flush(){
    if(fdatasync(handle) == 0)
        return true;
    else
        return false;
}
bool FLinuxHandle::Close(){
    if(close(handle) == 0)
        return true;
    else
        return false;
}
FLinuxHandle::~FLinuxHandle(){
    Close();
}

FLinuxLoader* FLinuxLoader::singleFLinuxLoader = nullptr;

FLinuxLoader* FLinuxLoader::GetFLinuxLoader(){
    if(singleFLinuxLoader == nullptr){
        static FLinuxLoader staticLinuxLoader;
        singleFLinuxLoader = &staticLinuxLoader;
    }
    return singleFLinuxLoader;
}

int64 FLinuxLoader::FileSize(const char* fileName){
    int64 filesize = -1;	
	struct stat statbuff;
	if(stat(fileName, &statbuff) < 0){
		return filesize;
	}else{
		filesize = statbuff.st_size;
	}
	return filesize;  
}

bool FLinuxLoader::FileExists(const char* fileName){
    int32 result = access(fileName, F_OK);
    if(result == 0)
        return true;
    else
        return false;
}

bool FLinuxLoader::FileDelete(const char* fileName){
    int32 result = remove(fileName);
    if(result == 0)
        return true;
    else
        return false;
}

bool FLinuxLoader::FileMove(const char* fileFrom, const char* fileTo){
    if(!FileExists(fileFrom))
        return false;
    int32 inHandle = open(fileFrom, O_RDONLY);
    int32 outHandle = open(fileTo, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
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

bool FLinuxLoader::FileCopy(const char* fileFrom, const char* fileTo){
    if(!FileExists(fileFrom))
        return false;
    int32 inHandle = open(fileFrom, O_RDONLY);
    int32 outHandle = open(fileTo, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
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

FHandle* FLinuxLoader::OpenRead(const char* fileName){
    if(FileExists(fileName)){
        int32 handle = open(fileName, O_RDONLY);
        if(handle == -1)
            return nullptr;
        FHandle* linuxHandle = new FLinuxHandle(handle, 0, FileSize(fileName));
        return linuxHandle;
    }
    return nullptr;
}

FHandle* FLinuxLoader::OpenWrite(const char* fileName, bool append){
    int32 handle;
    if(append)
        handle = open(fileName, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
    else
        handle = open(fileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
    if(handle == -1)
        return nullptr;
    FHandle* linuxHandle = new FLinuxHandle(handle, 0, FileSize(fileName));
    return linuxHandle;
}

bool FLinuxLoader::DirectoryExist(const char* directoryName){
    int32 result = access(directoryName, F_OK);
    if(result == 0)
        return true;
    else
        return false;
}

bool FLinuxLoader::DirectoryCreate(const char* directoryName){
    if(DirectoryExist(directoryName))
        return true;
    int result = mkdir(directoryName, S_IRWXU);
    if(result == 0)
        return true;
    else
        return false;
}

bool FLinuxLoader::DirectoryDelete(const char* directoryName){
    return true;
}

void FLinuxLoader::FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension){
    DIR* pDir = opendir(directory);
    dirent* pEnt;

    if(!pDir){
        DEBUG("open dir " << directory << " failed !");
        return;
    }

    while((pEnt = readdir(pDir)) != NULL){
        FString file = pEnt -> d_name;
        if(file.EndWith(extension)){
            foundFiles.PushBack(file);
        }
    }
}

void FLinuxLoader::FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension){

}

#endif
