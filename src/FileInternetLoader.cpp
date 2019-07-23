#include "FileInternetLoader.h"
#include "Hash.h"
#include "FilePakLoader.h"
#include <dirent.h>
#include <iostream>
#include <string>
#include <iomanip>


// int64 FInternetLoader::FileSize(const char* fileName){}

// // If the file is Existing
// bool FInternetLoader::FileExists(const char* fileName){}

// bool FInternetLoader::FileDelete(const char* fileName){}

// bool FInternetLoader::FileMove(const char* fileFrom, const char* fileTo){}

// bool FInternetLoader::FileCopy(const char* fileFrom, const char* fileTo){}

// /*
//     * Return read handle of file
//     * @param fileName  the name of file to read
//     * @return          the read handle of file
//     */
// FHandle* FInternetLoader::OpenRead(const char* fileName){}

// /*
//     * Return write handle of file
//     * @param fileName  the name of file to write
//     * @param append    if append to the end of file
//     * @return          the write handle of file
//     */
// FHandle* FInternetLoader::OpenWrite(const char* fileName, bool append){}

// // Directory operation
// bool FInternetLoader::DirectoryExist(const char* directoryName){}

// bool FInternetLoader::DirectoryCreate(const char* directoryName){}

// bool FInternetLoader::DirectoryDelete(const char* directoryName){}

// /*
//     * Find files with extension in the given directory
//     * @param foundFiles    array to save the files found
//     * @param directory     path to find files
//     * @param extension     file filter
//     */
// void FInternetLoader::FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension){}

//     /*
//     * Find files with extension in the given directory recursively
//     * @param foundFiles    array to save the files found
//     * @param directory     path to find files
//     * @param extension     file filter
//     */
// void FInternetLoader::FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension){}

FString defaultDir = "/home/wall/data/";
FString defaultDownloadExtension = ".downloading";

void Resume::PreLoad(){
  if(pos == 0){
        uint32 nameHash = Crc::MemCrc32(fileName.GetStr().c_str(), fileName.GetSize());
        FString downloadingName = FString(std::to_string(nameHash)) + defaultDownloadExtension;
        DEBUG("PreLoad downloadName : " << downloadingName.GetStr());
        DIR* pDir = opendir(defaultDir.GetStr().c_str());
        dirent* pEnt;

        if(!pDir){
            DEBUG("open dir " << defaultDir.GetStr() << " failed !");
            return;
        }

        while((pEnt = readdir(pDir)) != NULL){
            FString file = pEnt -> d_name;
            if(file == downloadingName){
                FString targetPath = defaultDir + downloadingName;
                FHandle* handle = physicalLoader -> OpenRead(targetPath.GetStr().c_str());
                uint32 tempVersion;
                uint64 tempSize;
                uint64 tempPos;
                int64 readSize = handle -> Read((uint8*)&tempVersion, sizeof(tempVersion));
                CMP_CHECK(readSize, sizeof(tempVersion), "Read downloading version failed !");
                readSize = handle -> Read((uint8*)&tempSize, sizeof(tempSize));
                CMP_CHECK(readSize, sizeof(tempSize), "Read downloading size failed !");
                readSize = handle -> Read((uint8*)&tempPos, sizeof(tempPos));
                CMP_CHECK(readSize, sizeof(tempPos), "Read downloading pos failed !");
                
                if(version == tempVersion && size == tempSize){
                    readSize = handle -> Read(data, tempPos);
                    CMP_CHECK(readSize, tempPos, "Read downloading data failed");
                    pos = tempPos;
                }

                physicalLoader -> FileDelete(targetPath.GetStr().c_str());
            }
        }
    }
}

void Resume::Print(){
    std::cout << std::left << std::setw(15) << "size" << size << std::endl;
    std::cout << std::left << std::setw(15) << "pos" << pos << std::endl;   
    std::cout << std::left << std::setw(15) << "FileName" << fileName.GetStr() << std::endl;
    std::cout << std::left << std::setw(15) << "version" << version << std::endl;
}

void Resume::WriteToDisk(){
    uint32 hashName = Crc::MemCrc32(fileName.GetStr().c_str(), fileName.GetSize());
    FString downloadingName = FString(std::to_string(hashName)) + defaultDownloadExtension;
    FString targetPath = defaultDir + downloadingName;
    DEBUG("WriteToDisk targetName : " << targetPath.GetStr())   ;
    Print();
    FHandle* handle = physicalLoader -> OpenWrite("/home/wall/data/334884854.downloading" , false);
    int writeSize = handle -> Write((uint8*)&version, sizeof(version));
    CMP_CHECK(writeSize, sizeof(version), "write downloading version failed !" << targetPath.GetStr());
    writeSize = handle -> Write((uint8*)&size, sizeof(size));
    CMP_CHECK(writeSize, sizeof(size), "write downloading size failed !" << targetPath.GetStr());
    writeSize = handle -> Write((uint8*)&pos, sizeof(pos));
    CMP_CHECK(writeSize, sizeof(pos), "write downloading pos failed !" << targetPath.GetStr());
    writeSize = handle -> Write(data, pos);
    CMP_CHECK(writeSize, pos, "write downloading data failed !" << targetPath.GetStr());
    DEBUG("Write Successfully !");
}

void WriteToPak(const uint8* data, uint64 byteToWrite, const char* fileName){
    FLinuxLoader* loader = FLinuxLoader::GetFWindowsLoader();
    FHandle* handle = loader -> OpenWrite("/home/wall/data/test.pak", false);
    FWriteArchive* wArchive = new FWriteArchive(handle, "/home/wall/data/test.pak", 0);

    PakFile pakFile;

   char a[4] = {'q', 'b', 'c', 'd'};
   char c[3] = {'1', '2', '3'};
   pakFile.SetVersion(1);
   pakFile.Write(handle, "123", (uint8*)a, 4);
   pakFile.Write(handle, "456", (uint8*)c, 3);

   pakFile.Serialize(*wArchive);

   wArchive -> Flush();

    FHandle* readHandle = loader -> OpenRead("/home/wall/data/test.pak");
    FReadArchive* rArchive = new FReadArchive(readHandle, "/home/wall/data/test.pak", readHandle -> Size());

    PakFile pakFile1;
    pakFile1.initialize(*rArchive);
    pakFile1.Write(handle, fileName, data, byteToWrite);
    handle -> Flush();

    PakEntry pakEntry;
    pakFile1.FindFile(fileName, pakEntry);
    uint8 p[byteToWrite];
    int readSize = pakFile1.Read(readHandle, pakEntry, p);
    DEBUG("read size : " << readSize);
    pakFile1.Print();
    for(int i = 0; i < byteToWrite; i++){
        std::cout << *(p+i);
    }
}

int64 Resume::Write(uint8* inBuffer, uint64 btyeToWrite){
    if(!inBuffer)
        return -1;
    if(pos + btyeToWrite > size)
        return -1;
    memcpy(data + pos, inBuffer, btyeToWrite);
    pos += btyeToWrite;

    if(pos == size){
        WriteToPak(data, size, fileName.GetStr().c_str());
        delete[] data;
        data = NULL;
    }
    return btyeToWrite;
}

Resume::~Resume(){
    if(data != NULL){
        WriteToDisk();
        delete[] data;
        data = NULL;
    }
}