#include "FilePakLoader.h"

void PakInfo::Serialize(FArchive& ar){
    ar << magic;
    if(magic != PkgMigic){
        magic = 0;
        return;
    }
    ar << indexOffset;  
    ar << indexSize;
}

int64 PakEntry::GetSerializedSize(){
    int64 retSize = sizeof(compressSize) + sizeof(uncompressSize) + sizeof(compressMethod) + \
        sizeof(offset) + sizeof(fBSize) + sizeof(flag);
    for(int i = 0; i < blockList.Size(); i++){
        retSize += blockList[i].GetSerializeSize();
    }
    return retSize;
}

void PakEntry::Serialize(FArchive& ar){
    ar << compressSize;
    ar << uncompressSize;
    ar << compressMethod;
    ar << offset;
    ar << fBSize;
    ar << flag;
    if(ar.IsRead()){
        for(int i = 0; i < blockList.Size(); i++){
            blockList[i].Serialize(ar);
        }
    }
    if(ar.IsWrite()){
        for(int i = 0; i < fBSize; i++){
            FileBlock fb(0, 0);
            fb.Serialize(ar);
            blockList.PushBack(fb);
        }
    }
}

void PakFile::FindFiles(FArray<FString> foundfiles, const char* directory, const char* extension){
    auto it = index.find(FString(directory));
    for(auto i = (it -> second).begin(); i != (it -> second).begin(); i++){
        FString s(i -> first);
        if(s.EndWith(extension))
            foundfiles.PushBack(s);
    }
}

void PakFile::FindFilesRecursively(FArray<FString> foundfiles, const char* directory, const char* extension){
    auto it = index.find(FString(directory));
    for(auto i = (it -> second).begin(); i != (it -> second).begin(); i++){
        FString s(i -> first);
        if(s.EndWith(extension)){
            foundfiles.PushBack(s);
            if(i -> second == -1){
                FString nextPath  = FString(directory) + "/" + (i -> first);
                FindFilesRecursively(foundfiles, nextPath.GetStr().c_str(), extension);
            }
        }
    }
}

bool PakFile::FindFile(const char* fileName, PakEntry* PakEntry){
    return true;
}

void PakFile::Serialize(FArchive& archive){
    if(archive.IsWrite()){
        info.Serialize(archive);
        archive.Seek(info.GetIndexOffset());
        for(int i = 0; i < files.Size(); i++){
            files[i].Serialize(archive);
        }
    }
    if(archive.IsRead()){

    }
}

FPakHandle::FPakHandle(FHandle* Inhandle, int64 InPos, PakFile& InPakFile, PakEntry& InPakEntry)
    : physicalHandle(Inhandle)
    , pakFile(InPakFile)
    , pakEntry(InPakEntry)
    , FHandle(InPos, InPakEntry.GetUnCompressSize())
{}
int64 FPakHandle::Read(uint8* inBuffer, int64 bytesToRead){ return -1;}
int64 FPakHandle::Write(const uint8* outBrffer, int64 bytesToWrite){ return -1;}

bool FPakHandle::Seek(int64 newPosition){
    newPosition += pakEntry.GetOffset();
    if(physicalHandle -> Seek(newPosition))
        return true;
    else
        return false;
}
bool FPakHandle::SeekFromEnd(int64 newPosition){
    newPosition += pakEntry.GetOffset() + pakEntry.GetUnCompressSize();
    if(physicalHandle -> SeekFromEnd(newPosition))
        return true;
    else
        return false;
}

bool FPakHandle::Flush(){
    if(physicalHandle -> Flush())
        return true;
    else
        return false;
}

bool FPakHandle::Close(){
    if(physicalHandle -> Close())
        return true;
    else
        return false;
}

FPakHandle::~FPakHandle(){
    if(physicalHandle)
        Close();
}

   
int64 FPakLoader::FileSize(const char* fileName){
    for(int i = 0; i < pakFiles.Size(); i++){
        PakEntry* pakEntry;
        if(pakFiles[i].FindFile(fileName, pakEntry)){
            return pakEntry -> GetUnCompressSize();
        }
    }
    return lowerFloader -> FileSize(fileName);
}

// If the file is Existing
bool FPakLoader::FileExists(const char* fileName){
    for(int i = 0; i < pakFiles.Size(); i++){
        PakEntry* pakEntry;
        if(pakFiles[i].FindFile(fileName, pakEntry)){
            return true;
        }
    }
    return lowerFloader -> FileSize(fileName);
}

bool FPakLoader::FileDelete(const char* fileName){
    return lowerFloader -> FileDelete(fileName);
}

bool FPakLoader::FileMove(const char* fileFrom, const char* fileTo){
    return lowerFloader -> FileMove(fileFrom, fileTo);
}

bool FPakLoader::FileCopy(const char* fileFrom, const char* fileTo){
    return lowerFloader -> FileCopy(fileFrom, fileTo);
}

/*
    * Return read handle of file
    * @param fileName  the name of file to read
    * @return          the read handle of file
    */
FHandle* FPakLoader::OpenRead(const char* fileName){
    for(int i = 0; i < pakFiles.Size(); i++){
        PakEntry* pakEntry;
        if(pakFiles[i].FindFile(fileName, pakEntry)){
            return new FPakHandle(GetPhysicalReadHandle(pakFiles[i].GetFileName().GetStr().c_str()), pakEntry -> GetOffset(), pakFiles[i], *pakEntry);
        }
    }
    return nullptr;

}

/*
    * Return write handle of file
    * @param fileName  the name of file to write
    * @param append    if append to the end of file
    * @return          the write handle of file
    */
FHandle* FPakLoader::OpenWrite(const char* fileName, bool append){
    for(int i = 0; i < pakFiles.Size(); i++){
        PakEntry* pakEntry;
        if(pakFiles[i].FindFile(fileName, pakEntry)){
            return new FPakHandle(GetPhysicalWriteHandle(pakFiles[i].GetFileName().GetStr().c_str()), pakEntry -> GetOffset(), pakFiles[i], *pakEntry);
        }
    }
    return nullptr;
}

// Directory operation
bool FPakLoader::DirectoryExist(const char* directoryName){
    return  false;
}

bool FPakLoader::DirectoryCreate(const char* directoryName){
    return false;
}

bool FPakLoader::DirectoryDelete(const char* directoryName){
    return false;
}

/*
    * Find files with extension in the given directory
    * @param foundFiles    array to save the files found
    * @param directory     path to find files
    * @param extension     file filter
    */
void FPakLoader::FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension){
   for(int i = 0; i < pakFiles.Size(); i++){
        pakFiles[i].FindFiles(foundFiles, directory, extension);
    }
}

    /*
    * Find files with extension in the given directory recursively
    * @param foundFiles    array to save the files found
    * @param directory     path to find files
    * @param extension     file filter
    */
void FPakLoader::FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension){
   for(int i = 0; i < pakFiles.Size(); i++){
        pakFiles[i].FindFilesRecursively(foundFiles, directory, extension);
    }
}