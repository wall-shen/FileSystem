#include "FilePakLoader.h"
#include "Hash.h"
#include <iomanip>

void PakInfo::Serialize(FArchive& ar){
    ar << magic;
    if(magic != PkgMigic){
        magic = 0;
        return;
    }
    ar << indexOffset;  
    ar << indexSize;
}

void PakInfo::Print(){
    std::cout << std::left << std::setw(15) << "magic" << magic << std::endl;
    std::cout << std::left << std::setw(15) << "indexOffset" << indexOffset << std::endl;
    std::cout << std::left << std::setw(15) << "indexSize" << indexSize << std::endl;
}

void FileBlock::Print(){
    std::cout << start << "  ->  " << end << std::endl;
}

int64 PakEntry::GetSerializedSize(){
    int64 retSize = sizeof(hashOne) + sizeof(hashTwo) + sizeof(compressSize) + sizeof(uncompressSize) + sizeof(compressMethod) + \
        sizeof(offset) + sizeof(fBSize) + sizeof(flag);
    for(int i = 0; i < blockList.Size(); i++){
        retSize += blockList[i].GetSerializeSize();
    }
    return retSize;
}

void PakEntry::Serialize(FArchive& ar){
    ar << hashOne;
    ar << hashTwo;
    ar << compressSize;
    ar << uncompressSize;
    ar << compressMethod;
    ar << offset;
    ar << fBSize;
    ar << flag;
    if(ar.IsRead()){
        for(int i = 0; i < fBSize; i++){
            FileBlock fb(0, 0);
            fb.Serialize(ar);
            blockList.PushBack(fb);
        }
    }
    if(ar.IsWrite()){
        for(int i = 0; i < blockList.Size(); i++){
            blockList[i].Serialize(ar);
        }
    }
}

void PakEntry::Print(){
    std::cout << std::left << std::setw(15) << "hashOne" << hashOne << std::endl;
    std::cout << std::left << std::setw(15) << "hashTwo" << hashTwo << std::endl;
    std::cout << std::left << std::setw(15) << "compressSize" << compressSize << std::endl;
    std::cout << std::left << std::setw(15) << "uncompressSize" << uncompressSize << std::endl;
    std::cout << std::left << std::setw(15) << "compressMethod" << compressMethod << std::endl;
    std::cout << std::left << std::setw(15) << "offset" << offset << std::endl;
    std::cout << std::left << std::setw(15) << "fbSize" << fBSize << std::endl;
    std::cout << std::left << std::setw(15) << "flag" << flag << std::endl;
    std::cout << std::left << std::setw(15) << "blockList info:" << std::endl;
    for(int i = 0; i < blockList.Size(); i++){
        blockList[i].Print();
    }
}

void PakFile::FindFiles(FArray<FString> foundfiles, const char* directory, const char* extension){
    
    // auto it = index.find(FString(directory));
    // for(auto i = (it -> second).begin(); i != (it -> second).begin(); i++){
    //     FString s(i -> first);
    //     if(s.EndWith(extension))
    //         foundfiles.PushBack(s);
    // }
}

void PakFile::FindFilesRecursively(FArray<FString> foundfiles, const char* directory, const char* extension){
    // auto it = index.find(FString(directory));
    // for(auto i = (it -> second).begin(); i != (it -> second).begin(); i++){
    //     FString s(i -> first);
    //     if(s.EndWith(extension)){
    //         foundfiles.PushBack(s);
    //         if(i -> second == -1){
    //             FString nextPath  = FString(directory) + "/" + (i -> first);
    //             FindFilesRecursively(foundfiles, nextPath.GetStr().c_str(), extension);
    //         }
    //     }
    // }
}

bool PakFile::FindFile(const char* fileName, PakEntry& pakEntry){
    if(!fileName)
        return false;
    uint32 hashOne = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueOne);
    uint32 hashTwo = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueTwo);
    auto range = index.equal_range(hashOne);
    for(auto it = range.first ; it != range.second; it++){
        if((it -> second).first == hashTwo){
            pakEntry = files[(it -> second).second];
            return true;
        }
    }
    return false;
}

int64 PakFile::Read(FHandle* handle, PakEntry& pakEntry, uint8* inBuffer){
    int copySize = 0;
    for(int i = 0; i < pakEntry.fBSize; i++){
        handle -> Seek((pakEntry.blockList)[i].GetStart());
        handle -> Read(inBuffer + copySize, (pakEntry.blockList)[i].GetSize());
    }
    return copySize;
}
int64 PakFile::Write(FHandle* handle, const char* fileName, const uint8* outBrffer, int64 bytesToWrite){
    if(!handle || !fileName || !outBrffer || bytesToWrite < 0)
        return -1;
    handle -> Seek(GetInfo().GetIndexOffset());
    int writeSize = handle -> Write(outBrffer, bytesToWrite);
    if(writeSize != bytesToWrite)
        return -1;
    uint32 hashOne = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueOne);
    uint32 hashTwo = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueTwo);

    // construct PakEntry
    PakEntry tempEntry;
    FileBlock tempBlock;
    files.PushBack(tempEntry);
    files[files.Size() - 1].hashOne = hashOne;
    files[files.Size() - 1].hashTwo = hashTwo;
    files[files.Size() - 1].compressSize = 0;
    files[files.Size() - 1].uncompressSize = bytesToWrite;
    files[files.Size() - 1].compressMethod = 0;
    files[files.Size() - 1].offset = GetInfo().GetIndexOffset();
    files[files.Size() - 1].fBSize = 1;
    files[files.Size() - 1].flag = 0x00;
    files[files.Size() - 1].blockList.PushBack(tempBlock);
    files[files.Size() - 1].blockList[0].start = GetInfo().GetIndexOffset();
    files[files.Size() - 1].blockList[0].end = GetInfo().GetIndexOffset() + bytesToWrite;

    info.indexOffset = files[files.Size() - 1].blockList[0].end;
    entryNum += 1;
    return bytesToWrite;
}

void PakFile::Serialize(FArchive& archive){
    // save to disk
    if(archive.IsWrite()){
        archive.Seek(info.GetIndexOffset());
        int64 indexSize = 0;
        for(int i = 0; i < files.Size(); i++){
            indexSize += files[i].GetSerializedSize();
            files[i].Serialize(archive);
        }
        info.indexSize = indexSize;
        archive.Seek(0);
        info.Serialize(archive);
    }

    // read from disk
    if(archive.IsRead()){
        archive.Seek(0);
        info.Serialize(archive);
        archive.Seek(info.GetIndexOffset());
        int64 indexSize = 0;
        while(indexSize < info.GetIndexSize()){
            files.PushBack(PakEntry());
            files[files.Size() - 1].Serialize(archive);
            indexSize += files[files.Size() - 1].GetSerializedSize();
        }
    }
}

void PakFile::initialize(FArchive& archive){
    Serialize(archive);
    pakFileName = archive.GetFileName();
    mountPoint = "/" + pakFileName.GetFileName();
    size = archive.Size();
    LoadIndex(archive);   
}

void PakFile::LoadIndex(FArchive& archive){
    if(!index.empty()){
        index.clear();
    }
    for(int i = 0; i < files.Size(); i++){
        index.insert(std::make_pair(files[i].hashOne, std::make_pair(files[i].hashTwo, i)));
        entryNum++;
    }
    SetIsValid(true);
}

void PakFile::Print(){
    info.Print();
    std::cout << std::left << std::setw(15) << "pakFileName" << pakFileName.GetStr() << std::endl;
    std::cout << std::left << std::setw(15) << "entryNum" << entryNum << std::endl;
    std::cout << std::left << std::setw(15) << "size" << size << std::endl;
    std::cout << std::left << std::setw(15) << "isValid" << isValid << std::endl;
    for(int i = 0; i < files.Size(); i++){
        std::cout << "the " << i << "th Entry info:" << std::endl;
        files[i].Print();
        std::cout << std::endl;
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
        PakEntry pakEntry;
        if(pakFiles[i].FindFile(fileName, pakEntry)){
            return pakEntry.GetUnCompressSize();
        }
    }
    return lowerFloader -> FileSize(fileName);
}

// If the file is Existing
bool FPakLoader::FileExists(const char* fileName){
    for(int i = 0; i < pakFiles.Size(); i++){
        PakEntry pakEntry;
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
        PakEntry pakEntry;
        if(pakFiles[i].FindFile(fileName, pakEntry)){
            return new FPakHandle(GetPhysicalReadHandle(pakFiles[i].GetFileName().GetStr().c_str()), pakEntry.GetOffset(), pakFiles[i], pakEntry);
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
        PakEntry pakEntry;
        if(pakFiles[i].FindFile(fileName, pakEntry)){
            return new FPakHandle(GetPhysicalWriteHandle(pakFiles[i].GetFileName().GetStr().c_str()), pakEntry.GetOffset(), pakFiles[i], pakEntry);
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