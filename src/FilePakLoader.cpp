#include "FilePakLoader.h"
#include "Hash.h"
#include <iomanip>
#include <zlib.h>

void PakInfo::Serialize(FArchive& ar){
    ar << magic;
    if(magic != PkgMigic){
        magic = 0;
        return;
    }
    ar << indexOffset;  
    ar << indexSize;
    ar << version;
}

void PakInfo::Print(){
    std::cout << std::left << std::setw(15) << "magic" << magic << std::endl;
    std::cout << std::left << std::setw(15) << "indexOffset" << indexOffset << std::endl;
    std::cout << std::left << std::setw(15) << "indexSize" << indexSize << std::endl;
    std::cout << std::left << std::setw(15) << "version" << version << std::endl;
}

void FileBlock::Print(){
    std::cout << start << "  ->  " << end << std::endl;
}

int64 PakEntry::GetSerializedSize(){
    int64 retSize = sizeof(version) + sizeof(hashOne) + sizeof(hashTwo) + sizeof(compressSize) + sizeof(uncompressSize) + sizeof(compressMethod) \
         + sizeof(maxBlockSize) + sizeof(fBSize) + sizeof(flag);
    for(int i = 0; i < blockList.Size(); i++){
        retSize += blockList[i].GetSerializeSize();
    }
    return retSize;
}

void PakEntry::Serialize(FArchive& ar){
    ar << version;
    ar << hashOne;
    ar << hashTwo;
    ar << compressSize;
    ar << uncompressSize;
    ar << compressMethod;
    ar << maxBlockSize;
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
    std::cout << std::left << std::setw(15) << "version" << version << std::endl;
    std::cout << std::left << std::setw(15) << "hashOne" << hashOne << std::endl;
    std::cout << std::left << std::setw(15) << "hashTwo" << hashTwo << std::endl;
    std::cout << std::left << std::setw(15) << "compressSize" << compressSize << std::endl;
    std::cout << std::left << std::setw(15) << "uncompressSize" << uncompressSize << std::endl;
    std::cout << std::left << std::setw(15) << "compressMethod" << compressMethod << std::endl;
    std::cout << std::left << std::setw(15) << "fbSize" << fBSize << std::endl;
    std::cout << std::left << std::setw(15) << "flag" << flag << std::endl;
    std::cout << std::left << std::setw(15) << "maxBlockSize" << maxBlockSize << std::endl;
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

bool PakFile::Remove(const char* fileName){
    if(!fileName)
        return false;
    uint32 hashOne = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueOne);
    uint32 hashTwo = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueTwo);
    auto range = index.equal_range(hashOne);
    for(auto it = range.first ; it != range.second; it++){
        if((it -> second).first == hashTwo){
            // set flag
            files[(it -> second).second].SetFlag(DeletedFlag);

            // add deleteList
            BlockList& tempBlockList = files[(it -> second).second].blockList;
            for(int i = 0; i < tempBlockList.Size(); i++){
                deleteList.insert(std::make_pair(tempBlockList[i].GetSize(), std::make_pair(i, (it -> second).second)));
            }

            // remove from fileIndex
            index.erase(it);
            entryNum--;
            return true;
        }
    }
    return false;
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
    uint64 unCompMaxSize = compressBound(pakEntry.uncompressSize);
    uint8 unCompressBuffer[unCompMaxSize];
    uint8 compressBuffer[pakEntry.compressSize];
    for(int i = 0; i < pakEntry.fBSize; i++){
        handle -> Seek((pakEntry.blockList)[i].GetStart());
        uint32 readSize = handle -> Read(compressBuffer + copySize, (pakEntry.blockList)[i].GetSize());
        if(readSize != (pakEntry.blockList)[i].GetSize()){
            DEBUG("pakFile::Read Read Block failed : " << pakEntry.blockList[i].start << " -> " << pakEntry.blockList[i].end);
            return -1;
        }
        copySize += readSize;
    }
    int32 err = uncompress(unCompressBuffer, &unCompMaxSize, compressBuffer, copySize);
    if(err != Z_OK){
        DEBUG("uncompress error : " << err);
        return -1;
    }
    memcpy(inBuffer, unCompressBuffer, unCompMaxSize);
    return unCompMaxSize;
}

int64 PakFile::WriteToBlock(FHandle* handle, uint64 startPos, const uint8* outBuffer, int64 byteToWrite){
    if(!handle || startPos < 0 || !outBuffer || byteToWrite < 0)
        return -1;
    if(!(handle -> Seek(startPos)))
        return -1;
    uint64 copySize = handle -> Write(outBuffer, byteToWrite);
    if(copySize != byteToWrite)
        return -1;
    return copySize;
}

bool PakFile::AddEntryToFiles(const char* fileName, uint64 compressSize, uint64 unCompressSize, uint32 compressMethod, BlockList& blockList){
    if(!fileName || compressSize < 0 || unCompressSize < 0 || CompressMethod < 0)
        return false;
    uint32 hashOne = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueOne);
    uint32 hashTwo = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueTwo);
    uint64 maxBlockSize = 0;
    for(int i = 0; i < blockList.Size(); i++){
        if(blockList[i].GetSize() > maxBlockSize)
            maxBlockSize = blockList[i].GetSize();
    }
    files.PushBack(PakEntry(info.version, hashOne, hashTwo, compressSize, unCompressSize, compressMethod, blockList, maxBlockSize, blockList.Size()));
    index.insert(std::make_pair(hashOne, std::make_pair(hashTwo, files.Size() - 1)));
    return true;
}

int64 PakFile::Write(FHandle* handle, const char* fileName, const uint8* outBuffer, int64 bytesToWrite){
    if(!handle || !fileName || !outBuffer || bytesToWrite < 0)
        return -1;
    Remove(fileName);

    // compress data
    int err;
    uint64 compressSize = compressBound(bytesToWrite);

    uint8 compressBuffer[compressSize];
    err = compress(compressBuffer, &compressSize, (const Bytef*)outBuffer, bytesToWrite);
    if(err != Z_OK){
        DEBUG("compress error : " << err);
        return -1;
    }

    //write data
    BlockList newEntryBlockList;
    auto deleteListIt = deleteList.lower_bound(compressSize);
    if(deleteListIt != deleteList.end()){

        /**
         * areaPos -> first     offset of block in blockList of entry
         * areaPos -> second    offset of entry in files
         */
        auto areaPos = deleteListIt -> second;
        FileBlock targetBlock = files[areaPos.second].blockList[areaPos.first];
        int CopySize = WriteToBlock(handle, targetBlock.start, compressBuffer, compressSize);

        if(CopySize != compressSize){
            DEBUG("write error : " << targetBlock.start << " -> " << targetBlock.end);
            return -1;
        }

        if(compressSize == targetBlock.GetSize()){
            deleteList.erase(deleteListIt);
            if(files[areaPos.second].blockList.Size() == 1){
                files.Erase(areaPos.second);
            }
            else{
                files[areaPos.second].blockList.Erase(areaPos.first);
            }
        }
        else{
            deleteList.erase(deleteListIt);
            files[areaPos.second].blockList[areaPos.first].start += compressSize;
            targetBlock.end = files[areaPos.second].blockList[areaPos.first].start;
            deleteList.insert(std::make_pair(files[areaPos.second].blockList[areaPos.first].GetSize(), std::make_pair(areaPos.first, areaPos.second)));
        }
        newEntryBlockList.PushBack(targetBlock);
    }
    else{
        uint64 CopySize = 0;
        uint64 leftToWrite = compressSize;
        if(deleteList.size() != 0){
            /**
             * areaPos -> first     offset of block in blockList of entry
             * areaPos -> second    offset of entry in files
             */
            auto areaPos = (--deleteListIt) -> second;
            FileBlock& targetBlock = files[areaPos.second].blockList[areaPos.first];
            CopySize = WriteToBlock(handle, targetBlock.start, compressBuffer, targetBlock.GetSize());
            newEntryBlockList.PushBack(targetBlock);
            leftToWrite -= CopySize;

            if(CopySize != targetBlock.GetSize()){
                DEBUG("write error : " << targetBlock.start << " -> " << targetBlock.end);
                return -1;
            }

            deleteList.erase(deleteListIt);
            if(files[areaPos.second].blockList.Size() == 1){
                files.Erase(areaPos.second);
            }
            else{
                files[areaPos.second].blockList.Erase(areaPos.first);
            }
        }

        // write to new block
        uint64 copySizeTwo = WriteToBlock(handle, GetInfo().GetIndexOffset(), compressBuffer + CopySize, leftToWrite);
        newEntryBlockList.PushBack(FileBlock(GetInfo().GetIndexOffset(), GetInfo().GetIndexOffset() + copySizeTwo));
        if(copySizeTwo != leftToWrite){
            DEBUG("write error : " << GetInfo().GetIndexOffset() << " -> " << (GetInfo().GetIndexOffset() + copySizeTwo));
            return -1;
        }

        info.indexOffset += copySizeTwo;
    }

    if(!AddEntryToFiles(fileName, compressSize, bytesToWrite, 0, newEntryBlockList))
        return -1;
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
    entryNum = 0;
    if(!index.empty()){
        index.clear();
    }
    for(int i = 0; i < files.Size(); i++){
        if(files[i].flag == DeletedFlag){
            for(int j = 0; j < files[i].blockList.Size(); j++){
                FileBlock& targetBlock = files[i].blockList[j];
                deleteList.insert(std::make_pair(targetBlock.GetSize(), std::make_pair(j, i)));
            }
        }
        else{
            index.insert(std::make_pair(files[i].hashOne, std::make_pair(files[i].hashTwo, i)));
        }
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
    newPosition += pakEntry.blockList[0].start;
    if(physicalHandle -> Seek(newPosition))
        return true;
    else
        return false;
}
bool FPakHandle::SeekFromEnd(int64 newPosition){
    newPosition += pakEntry.blockList[0].start + pakEntry.GetUnCompressSize();
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


FPakLoader::FPakLoader(const char* loaderDir){
#ifdef LINUX
    physicalLoader = FLinuxLoader::GetFLinuxLoader();
#endif

#ifdef WINDOWS
    physicalLoader = FWindowsLoader::GetFWindowsLoader();
#endif
    FArray<FString> files;
    physicalLoader -> FindFiles(files, loaderDir, PAK_EXTENSION);
    for(int i = 0; i < files.Size(); i++){
        pakFiles.PushBack(PakFile());
        DEBUG((FString(loaderDir) + files[i]).GetStr().c_str());
        FReadArchive* rArchive = new FReadArchive((FString(loaderDir) + files[i]).GetStr().c_str());
        pakFiles[i].initialize(*rArchive);
        delete rArchive;
    }
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
            return new FPakHandle(GetPhysicalReadHandle(pakFiles[i].GetFileName().GetStr().c_str()), pakEntry.blockList[0].start, pakFiles[i], pakEntry);
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
            return new FPakHandle(GetPhysicalWriteHandle(pakFiles[i].GetFileName().GetStr().c_str()), pakEntry.blockList[0].start, pakFiles[i], pakEntry);
        }
    }
    return nullptr;
}

// Directory operation
bool FPakLoader::DirectoryExist(const char* directoryName){
    return lowerFloader -> DirectoryExist(directoryName);
}

bool FPakLoader::DirectoryCreate(const char* directoryName){
    return lowerFloader -> DirectoryCreate(directoryName);
}

bool FPakLoader::DirectoryDelete(const char* directoryName){
    return lowerFloader -> DirectoryDelete(directoryName);
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

FPakLoader::~FPakLoader(){
    for(int i = 0; i < pakFiles.Size(); i++){
        FWriteArchive* wArchive = new FWriteArchive(pakFiles[i].pakFileName.GetStr().c_str(), 0);
        pakFiles[i].Serialize(*wArchive);
        delete wArchive;
    }
}