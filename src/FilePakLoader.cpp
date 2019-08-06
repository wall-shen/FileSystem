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
    int64 retSize = sizeof(hashOne) + sizeof(hashTwo) + sizeof(compressSize) + sizeof(uncompressSize) + sizeof(compressMethod) \
         + sizeof(maxBlockSize) + sizeof(fBSize) + sizeof(flag) + md5.GetSerializedSize();
    for(int i = 0; i < blockList.Size(); i++){
        retSize += blockList[i].GetSerializeSize();
    }
    return retSize;
}

void PakEntry::Serialize(FArchive& ar){
    ar << hashOne;
    ar << hashTwo;
    ar << md5;
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
    std::cout << std::left << std::setw(15) << "hashOne" << hashOne << std::endl;
    std::cout << std::left << std::setw(15) << "hashTwo" << hashTwo << std::endl;
    std::cout << std::left << std::setw(15) << "md5" << md5.GetStr() << std::endl;
    std::cout << std::left << std::setw(15) << "compressSize" << compressSize << std::endl;
    std::cout << std::left << std::setw(15) << "uncompressSize" << uncompressSize << std::endl;
    std::cout << std::left << std::setw(15) << "compressMethod" << compressMethod << std::endl;
    std::cout << std::left << std::setw(15) << "fbSize" << fBSize << std::endl;
    if(flag == NormalFlag)
        std::cout << std::left << std::setw(15) << "flag" << "normal" << std::endl;
    else if(flag == DeletedFlag)
        std::cout << std::left << std::setw(15) << "flag" << "delete" << std::endl;
    else if(flag == DownloadFlag)
        std::cout << std::left << std::setw(15) << "flag" << "download" << std::endl;
    else if(flag == DiscardFlag)
        std::cout << std::left << std::setw(15) << "flag" << "discard" << std::endl;
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

bool PakFile::FindFileWithEntryIndex(const char* fileName, PakEntry& pakEntry, int32& entryIndex){
    if(!fileName)
        return false;
    uint32 hashOne = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueOne);
    uint32 hashTwo = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueTwo);
    auto range = index.equal_range(hashOne);
    for(auto it = range.first ; it != range.second; it++){
        if((it -> second).first == hashTwo){
            pakEntry = files[(it -> second).second];
            entryIndex = (it -> second).second;
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

bool PakFile::AddEntryToFiles(const char* fileName, uint64 compressSize, uint64 unCompressSize, uint32 compressMethod, BlockList& blockList, FString md5){
    if(!fileName || compressSize < 0 || unCompressSize < 0 || CompressMethod < 0)
        return false;
    uint32 hashOne = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueOne);
    uint32 hashTwo = Crc::MemCrc32(fileName, strlen(fileName), Crc::crcValueTwo);
    uint64 maxBlockSize = 0;
    for(int i = 0; i < blockList.Size(); i++){
        if(blockList[i].GetSize() > maxBlockSize)
            maxBlockSize = blockList[i].GetSize();
    }
    files.PushBack(PakEntry(hashOne, hashTwo, compressSize, unCompressSize, compressMethod, blockList, maxBlockSize, blockList.Size(), md5, DownloadFlag));
    index.insert(std::make_pair(hashOne, std::make_pair(hashTwo, files.Size() - 1)));
    return true;
}

int64 PakFile::Write(FHandle* handle, const char* fileName, const uint8* outBuffer, int64 bytesToWrite, FString md5){
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
                files[areaPos.second].fBSize--;
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
                files[areaPos.second].fBSize--;
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

    if(!AddEntryToFiles(fileName, compressSize, bytesToWrite, 0, newEntryBlockList, md5))
        return -1;
    entryNum += 1;
    return bytesToWrite;
}

int64 PakFile::CreateFile(const char* fileName, int64 size, int64 uncompressSize, uint32 compressMethod, FString md5){
    if(!fileName || size <= 0)
        return -1;
    Remove(fileName);


    //alloc rom
    BlockList newEntryBlockList;
    auto deleteListIt = deleteList.lower_bound(size);
    if(deleteListIt != deleteList.end()){

        /**
         * areaPos -> first     offset of block in blockList of entry
         * areaPos -> second    offset of entry in files
         */
        auto areaPos = deleteListIt -> second;
        FileBlock targetBlock = files[areaPos.second].blockList[areaPos.first];

        if(size == targetBlock.GetSize()){
            deleteList.erase(deleteListIt);
            files[areaPos.second].blockList[areaPos.first].start = 0;
            files[areaPos.second].blockList[areaPos.first].end = 0;
            BlockList& list = files[areaPos.second].blockList;
            for(int i = 0; i < list.Size(); i++){
                if(list[i].GetSize() != 0){
                    break;
                }
                if(i == list.Size() - 1){
                    files[areaPos.second].flag = DiscardFlag;
                }
            }
            // if(files[areaPos.second].blockList.Size() == 1){
            //     files.Erase(areaPos.second);
            // }
            // else{
            //     files[areaPos.second].blockList.Erase(areaPos.first);
            //     files[areaPos.second].fBSize--;
            // }
        }
        else{
            deleteList.erase(deleteListIt);
            files[areaPos.second].blockList[areaPos.first].start += size;
            targetBlock.end = files[areaPos.second].blockList[areaPos.first].start;
            deleteList.insert(std::make_pair(files[areaPos.second].blockList[areaPos.first].GetSize(), std::make_pair(areaPos.first, areaPos.second)));
        }
        newEntryBlockList.PushBack(targetBlock);
    }
    else{
        uint64 leftToWrite = size;
        if(deleteList.size() != 0){
            /**
             * areaPos -> first     offset of block in blockList of entry
             * areaPos -> second    offset of entry in files
             */
            auto areaPos = (--deleteListIt) -> second;
            FileBlock& targetBlock = files[areaPos.second].blockList[areaPos.first];
            newEntryBlockList.PushBack(targetBlock);
            leftToWrite -= targetBlock.GetSize();

            deleteList.erase(deleteListIt);
            files[areaPos.second].blockList[areaPos.first].start = 0;
            files[areaPos.second].blockList[areaPos.first].end = 0;
            BlockList& list = files[areaPos.second].blockList;
            for(int i = 0; i < list.Size(); i++){
                if(list[i].GetStart() != 0 || list[i].GetEnd() != 0){
                    break;
                }
                if(i == list.Size() - 1){
                    files[areaPos.second].flag = DiscardFlag;
                }
            }
            // if(files[areaPos.second].blockList.Size() == 1){
            //     files.Erase(areaPos.second);
            // }
            // else{
            //     files[areaPos.second].blockList.Erase(areaPos.first);
            //     files[areaPos.second].fBSize--;
            // }
        }

         // alloc second chunk
        auto secDeleteListIt = deleteList.lower_bound(leftToWrite);
        if(secDeleteListIt != deleteList.end()){

            /**
            * areaPos -> first     offset of block in blockList of entry
            * areaPos -> second    offset of entry in files
            */
            auto areaPos = secDeleteListIt -> second;
            FileBlock targetBlock = files[areaPos.second].blockList[areaPos.first];

            if(leftToWrite == targetBlock.GetSize()){
                deleteList.erase(secDeleteListIt);
                files[areaPos.second].blockList[areaPos.first].start = 0;
                files[areaPos.second].blockList[areaPos.first].end = 0;
                BlockList& list = files[areaPos.second].blockList;
                for(int i = 0; i < list.Size(); i++){
                    if(list[i].GetStart() != 0 || list[i].GetEnd() != 0){
                        break;
                    }
                    if(i == list.Size() - 1){
                        files[areaPos.second].flag = DiscardFlag;
                    }
                }
                // if(files[areaPos.second].blockList.Size() == 1){
                //     files.Erase(areaPos.second);
                // }
                // else{
                //     files[areaPos.second].blockList.Erase(areaPos.first);
                //     files[areaPos.second].fBSize--;
                // }
            }
            else{
                deleteList.erase(secDeleteListIt);
                files[areaPos.second].blockList[areaPos.first].start += leftToWrite;
                targetBlock.end = files[areaPos.second].blockList[areaPos.first].start;
                deleteList.insert(std::make_pair(files[areaPos.second].blockList[areaPos.first].GetSize(), std::make_pair(areaPos.first, areaPos.second)));
            }
            newEntryBlockList.PushBack(targetBlock);
        }
        else{
            // write to new block
            newEntryBlockList.PushBack(FileBlock(GetInfo().GetIndexOffset(), GetInfo().GetIndexOffset() + leftToWrite));

            info.indexOffset += leftToWrite;
        }

    }
    
    if(!AddEntryToFiles(fileName, 0, uncompressSize, compressMethod, newEntryBlockList, md5))
        return -1;
    entryNum += 1;
    return size;
}

void PakFile::Serialize(FArchive& archive){
    // save to disk
    if(archive.IsWrite()){
        archive.Seek(info.GetIndexOffset());
        int64 indexSize = 0;
        for(int i = 0; i < files.Size(); i++){
            if(files[i].flag != DiscardFlag){
                indexSize += files[i].GetSerializedSize();
                files[i].Serialize(archive);                
            }
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
    size = archive.Size();
    LoadIndex(archive);
}

void PakFile::LoadIndex(FArchive& archive){
    entryNum = 0;
    if(!index.empty()){
        index.clear();
    }
    if(!deleteList.empty()){
        deleteList.clear();
    }
    for(int i = 0; i < files.Size(); i++){
        if(files[i].flag == DeletedFlag){
            for(int j = 0; j < files[i].blockList.Size(); j++){
                FileBlock& targetBlock = files[i].blockList[j];
                if(targetBlock.GetSize() != 0)
                    deleteList.insert(std::make_pair(targetBlock.GetSize(), std::make_pair(j, i)));
            }
        }
        else if(files[i].flag == NormalFlag || files[i].flag == DownloadFlag){
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
    , blockListIndex(0)
    , entrySize(0)
    , data(nullptr)
    , FHandle(InPos, InPakEntry.GetUnCompressSize())
{
    physicalHandle -> Seek(pakEntry.blockList[0].start);

    for(int i = 0; i < pakEntry.blockList.Size(); i++){
        entrySize += pakEntry.blockList[i].GetSize();
    }
}

int64 FPakHandle::Read(uint8* inBuffer, int64 bytesToRead){
    if(data == nullptr){
        size = pakEntry.uncompressSize;
        data = new uint8[size];
        BlockList& list = pakEntry.blockList;
        int64 copySize = 0;
        for(int i = 0; i < list.Size(); i++){
            physicalHandle -> Seek(list[i].GetStart());
            int64 read = physicalHandle -> Read(data + copySize, list[i].GetSize());
            if(read != list[i].GetSize()){
                DEBUG("FPakHandle::Read read from pak failed");
                return -1;
            }
            else{
                copySize += read;
            }
        }
    }
    if(!inBuffer || bytesToRead < 0)
        return -1;
    if(bytesToRead >= size - pos){
        memcpy(inBuffer, data + pos, bytesToRead);
        pos += bytesToRead;
        return bytesToRead;
    }
    else{
        int64 readSize = size = pos;
        memcpy(inBuffer, data + pos, readSize);
        pos = size;
        return readSize;
    }
}

bool FPakHandle::CompareMd5(){
    int64 ReadSize = pakEntry.uncompressSize;
    data = new uint8[ReadSize];
    BlockList& list = pakEntry.blockList;
    int64 copySize = 0;
    FPhysicalLoader* phyLoader = FLinuxLoader::GetFLinuxLoader();
    DEBUG(pakFile.pakFileName.GetStr().c_str());
    FHandle* readHandle = phyLoader -> OpenRead(pakFile.pakFileName.GetStr().c_str());
    for(int i = 0; i < list.Size(); i++){
        if(!readHandle -> Seek(list[i].GetStart())){
            DEBUG("FPakHandle Seek Failed");
        }
        int64 read = readHandle -> Read(data + copySize, list[i].GetSize());
        DEBUG("FPakHandle::CompareMd5 read size " << read);
        if(read != list[i].GetSize()){
            DEBUG("FPakHandle::Read read from pak failed");
            return false;
        }
        else{
            copySize += read;
        }
    }
    uint8 result[16];
    MD5_CTX md5;
    MD5Init(&md5);
    MD5Update(&md5, (uint8*)data, ReadSize);
    MD5Final(&md5, result);
    char re[17];
    memcpy((char*)re, (char*)result, 16);
    re[16] = '\0';
    FString md5Str = re;
    if(md5Str == pakEntry.md5)
        return true;
    else{
        DEBUG(pakEntry.hashOne << " compare md5 failed!");
        return false;
    }
}

int64 FPakHandle::Write(const uint8* outBuffer, int64 bytesToWrite){
    if(!outBuffer || bytesToWrite > entrySize - pos)
        return -1;
    BlockList& list = pakEntry.blockList;
    int64 writeSize = 0;
    while(bytesToWrite > writeSize){
        int64 leftToWrite = bytesToWrite - writeSize;
        int64 physicalPos = physicalHandle -> Tell();
        int64 leftCanWrite = list[blockListIndex].GetEnd() - physicalPos;
        if(leftToWrite <= leftCanWrite){
            int64 copySize = physicalHandle -> Write(outBuffer + writeSize, leftToWrite);
            if(copySize != leftToWrite){
                DEBUG("FPakHandle::Write Failed, pos : " << physicalPos << " , writeSize : " << leftToWrite);
                return -1;
            }
            pos += copySize;
            writeSize += copySize;
            Seek(pos);
        }
        else{
            int64 copySize = physicalHandle -> Write(outBuffer + writeSize, leftCanWrite);
            if(copySize != leftCanWrite){
                DEBUG("FPakHandle::Write Failed, pos : " << physicalPos << " , writeSize : " << leftCanWrite);
                return -1;
            }
            pos += copySize;
            writeSize += copySize;
            blockListIndex++;
            physicalHandle -> Seek(list[blockListIndex].GetStart());
        }
    }
    physicalHandle -> Flush();
    pakEntry.compressSize += writeSize;
    if(pakEntry.compressSize == entrySize){
            pakEntry.flag = NormalFlag;
    }
    return writeSize;
}

bool FPakHandle::Seek(int64 newPosition){
    if(newPosition < 0)
        return false;
    BlockList& list = pakEntry.blockList;
    if(newPosition == size){
        if(physicalHandle -> Seek(list[list.Size() - 1].GetEnd())){
            blockListIndex = list.Size() - 1;
            return true;
        }
        else{
            DEBUG("FPakHandle::Seek physical failed, from : " << list[list.Size() - 1].GetEnd() << " , pos : " << newPosition);
            return false;
        }
    }
    for(int i = 0; i < list.Size(); i++){
        if(list[i].GetSize() <= newPosition){
            newPosition -= list[i].GetSize();
        }
        else{
            if(physicalHandle -> Seek(list[i].GetStart() + newPosition)){
                blockListIndex = i;
                return true;
            }
            else{
                DEBUG("FPakHandle::Seek physical failed, from : " << list[i].GetStart() << " , pos : " << newPosition);
                return false;
            }
        }
    }
    DEBUG("FPakHandle::Seek out of block range");
    return false;
}
bool FPakHandle::SeekFromEnd(int64 newPosition){
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
    if(data){
        delete data;
        data = nullptr;
    }
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
        FReadArchive* rArchive = new FReadArchive((FString(loaderDir) + files[i]).GetStr().c_str());
        pakFiles[i].initialize(*rArchive);
        delete rArchive;
    }
}
FPakLoader* FPakLoader::singleFPakLoader = nullptr;
FPakLoader* FPakLoader::GetFPakLoader(){
    if(singleFPakLoader == nullptr){
        static FPakLoader staticFakLoader;
        singleFPakLoader = &staticFakLoader;
    }
    return singleFPakLoader;
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
        int entryIndex = 0;
        if(pakFiles[i].FindFileWithEntryIndex(fileName, pakEntry, entryIndex)){
            //if(pakEntry.flag == NormalFlag)
                return new FPakHandle(GetPhysicalReadHandle(pakFiles[i].GetFileName().GetStr().c_str()), 0, pakFiles[i], pakFiles[i].files[entryIndex]);
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
        int entryIndex = 0;
        if(pakFiles[i].FindFileWithEntryIndex(fileName, pakEntry, entryIndex)){
            FHandle* phyHandle = GetPhysicalWriteHandle(pakFiles[i].GetFileName().GetStr().c_str());
            if(!phyHandle){
                DEBUG("pakLoader OpenWrite create phyHandle failed");
                return nullptr;
            }
            return new FPakHandle(phyHandle, 0, pakFiles[i], pakFiles[i].files[entryIndex]);
        }
    }
    return nullptr;
}

int64 FPakLoader::Write(const char * InpakName, const char* fileName, const uint8* outBuffer, int64 bytesToWrite, FString md5){
    if(!InpakName || !fileName || !outBuffer || bytesToWrite < 0)
        return -1;

    FString pakName = InpakName;
    for(int i = 0; i < pakFiles.Size(); i++){
        if(pakName == pakFiles[i].GetFileName()){
            FHandle* handle = GetPhysicalWriteHandle(pakFiles[i].GetFileName().GetStr().c_str());
            int64 copySize = pakFiles[i].Write(handle, fileName, outBuffer, bytesToWrite, md5);
            delete handle;
            return copySize;
        }
    }
    return 0;
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

void FPakLoader::FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension){
   for(int i = 0; i < pakFiles.Size(); i++){
        pakFiles[i].FindFiles(foundFiles, directory, extension);
    }
}

void FPakLoader::FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension){
   for(int i = 0; i < pakFiles.Size(); i++){
        pakFiles[i].FindFilesRecursively(foundFiles, directory, extension);
    }
}

bool FPakLoader::CreatePak(const char* pakName){
    for(int i = 0; i < pakFiles.Size(); i++){
        if(pakFiles[i].pakFileName == pakName){
            return false;
        }
    }
    FHandle* handle = physicalLoader -> OpenWrite(pakName, false);
    if(!handle)
        return false;
    FWriteArchive* wArchive = new FWriteArchive(handle, pakName, 0);
    pakFiles.PushBack(PakFile(pakName));
    int32 pos = pakFiles.Size() - 1;
    pakFiles[pos].Serialize(*wArchive);
    wArchive -> Flush();

    delete wArchive;
    return true;
}


int64 FPakLoader::Compare(const char* fileName, FString md5, int64 size){
    for(int i = 0; i < pakFiles.Size(); i++){
        PakEntry pakEntry;
        int entryIndex = 0;
        if(pakFiles[i].FindFileWithEntryIndex(fileName, pakEntry, entryIndex)){
            if(pakEntry.getFlag() == NormalFlag && pakEntry.md5 == md5 && pakEntry.GetCompressSize() == size){
                return size;
            }
            else if(pakEntry.getFlag() == DownLoadFile && pakEntry.md5 == md5){
                return pakEntry.GetCompressSize();
            }
        }
    }
    return -1;
}

int64 FPakLoader::CreateEntry(const char* fileName, int64 compressSize, int64 unCompressSize, uint32 compressMethod, FString md5){
    if(pakFiles.Size() != 0){
        return pakFiles[0].CreateFile(fileName, compressSize, unCompressSize, compressMethod, md5);
    }
    return -1;
}

bool FPakLoader::ReSize(FString pakName){
    for(int i = 0; i < pakFiles.Size(); i++){
        if(pakFiles[i].pakFileName == pakName){
            FString newPakName = pakName + "~";
            CreatePak(newPakName.GetStr().c_str());

            FHandle* readHandle = physicalLoader -> OpenRead(pakName.GetStr().c_str());
            FHandle* writeHandle = physicalLoader -> OpenWrite(newPakName.GetStr().c_str(), false);
            PakFile& rePak = pakFiles[i];
            PakFile& tarPak = pakFiles[pakFiles.Size() -1];
            writeHandle -> Seek(tarPak.info.GetSerializedSize());

            for(int j = 0; j < rePak.files.Size() && rePak.files[j].flag == NormalFlag; j++){
                int64 startPos = writeHandle -> Tell();
                BlockList& list = rePak.files[j].blockList;
                uint8 data[rePak.files[j].compressSize];
                int64 copySize = 0;
                for(int m = 0; m < list.Size(); m++){
                    readHandle -> Seek(list[m].GetStart());
                    int64 readSize = readHandle -> Read(data + copySize, list[i].GetSize());
                    copySize += readSize;
                }
                int64 writeSize = writeHandle -> Write(data, copySize);
                if(writeSize != copySize){
                    DEBUG("ReSize write Failed, pos " << writeHandle -> Tell());
                    return false;
                }
                int64 endPos = writeHandle -> Tell();
                BlockList newList;
                newList.PushBack(FileBlock(startPos, endPos));
                PakEntry& entry = rePak.files[j];
                tarPak.files.PushBack(PakEntry(entry.hashOne, entry.hashTwo, entry.compressSize, entry.uncompressSize, entry.compressMethod, newList, copySize, newList.Size(), entry.md5, NormalFlag));
                tarPak.index.insert(std::make_pair(entry.hashOne, std::make_pair(entry.hashTwo, tarPak.files.Size() - 1)));
            }
            if(readHandle){
                delete readHandle;
                readHandle = nullptr;
            }
            if(writeHandle){
                delete writeHandle;
                writeHandle = nullptr;
            }
            return true;
        }
    }
    return false;
}

FPakLoader::~FPakLoader(){
    for(int i = 0; i < pakFiles.Size(); i++){
        FWriteArchive* wArchive = new FWriteArchive(pakFiles[i].pakFileName.GetStr().c_str(), 0);
        pakFiles[i].Serialize(*wArchive);
        delete wArchive;
    }
}

void Combine(){
    FString dir = "/home/wall/data/temp/";
    DIR* pDir = opendir(dir.GetStr().c_str());
    dirent* entry;

    if(pDir == nullptr){
        DEBUG("combine opendir failed");
        return;
    }
    int count = 0;
    while((entry = readdir(pDir)) != nullptr){
        FString fileName = entry -> d_name;
        if(fileName.EndWith("downloading")){
            count ++;
            FLinuxLoader* phyLoader = FLinuxLoader::GetFLinuxLoader();
            FPakLoader* pakLoader = FPakLoader::GetFPakLoader();

            FString targetPath = dir + fileName;
            FHandle* readHandle = phyLoader -> OpenRead(targetPath.GetStr().c_str());
            int64 fileSize = phyLoader -> FileSize(targetPath.GetStr().c_str());
            uint8 data[fileSize];
            int64 readSize = readHandle -> Read(data, fileSize);
            if(readSize != fileSize){
                DEBUG("combine read failed");
                return;
            }
            DEBUG(targetPath.GetStr() << " has read " << readSize << " byte");

            FString pakName = fileName.GetFileNameWithoutExtension();
            FHandle* writeHandle = pakLoader -> OpenWrite(pakName.GetStr().c_str());
            int64 writeSize = writeHandle -> Write(data, fileSize);
            if(writeSize != fileSize){
                DEBUG("combine write failed");
                return;
            }

            DEBUG(targetPath.GetStr() << " has write " << writeSize << " byte");
            writeHandle -> Flush();
            delete writeHandle;
            delete readHandle;

        
        }
    }
    DEBUG("total read " << count << " file");
}