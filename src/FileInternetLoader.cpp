#include "FileInternetLoader.h"
#include "Hash.h"
#include "FilePakLoader.h"
#include <dirent.h>
#include <iostream>
#include <string>
#include <iomanip>

FInternetHandle::FInternetHandle(const char* InIp, const char* InfileName, int64 InPos, int64 InSize)
    : ip(InIp)
    , fileName(InfileName)
    , FHandle(InPos, InSize)
{
    curl = curl_easy_init();
}


class Memory{
public:
    uint8* memory;
    size_t size;

    Memory()
        : memory(nullptr)
        , size(0)
    {
        memory = (uint8*)malloc(1);
        if(memory == nullptr){
            DEBUG("Memory malloc failed!");
        }
    }

    ~Memory(){
        if(memory){
            free(memory);
            memory = nullptr;
        }
    }
};

size_t readFromData(void* buffer, size_t size, size_t nmemb, void* userPtr){
    size_t realsize = size * nmemb;
    Memory* mem = (Memory*)userPtr;

    if(mem -> memory == nullptr){
        DEBUG("readFromData memory is nullptr");
        return 0;
    }
    mem->memory = (uint8*)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        //out of memory!
        DEBUG("read From data not enough memory (realloc returned NULL)n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), buffer, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
    // json = cJSON_Parse((char*)buffer);
    // if(!json){
    //     return size * nmemb;
    // }
    // jsonLength = cJSON_GetObjectItem(json, "length");
    // jsonSize = cJSON_GetObjectItem(json, "size");
    // jsonData = cJSON_GetObjectItem(json, "data");
    // ReadInfo* data = (ReadInfo*)userPtr;
    // data -> size = jsonSize -> valueint;
    // data -> length = jsonLength -> valueint;
    // memcpy(data -> data, jsonData -> valuestring, data -> length);
    // if(json)
    //     cJSON_Delete(json);
    // return size * nmemb; 
}

int64 FInternetHandle::Read(uint8* inBuffer, int64 bytesToRead){
    if(!curl)
    return -1;
    Memory chunk;
    FString url = FString(ip) + "/download?fileName=" + FString(fileName) + "&pos=" + FString(std::to_string(pos)) + "&length=" + FString(std::to_string(bytesToRead));
    DEBUG(url.GetStr());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.GetStr().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, readFromData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    // setup keep-alive
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    // keep-alive idle time to 120 seconds */
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
    // interval time between keep-alive probes: 60 seconds */
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);

    curl_easy_perform(curl);
    long retCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retCode);

    cJSON *json, *jsonLength, *jsonSize, *jsonData;
    if(retCode == 200){
        json = cJSON_Parse((char*)chunk.memory);
        if(!json){
            DEBUG("Internet Read CJOSN failed");
            return -1;
        }

        jsonLength = cJSON_GetObjectItem(json, "length");
        jsonData = cJSON_GetObjectItem(json, "data");
        memcpy(inBuffer, jsonData -> valuestring, jsonLength -> valueint);
        if(json)
            cJSON_Delete(json);

        pos += jsonLength -> valueint;
        return jsonLength -> valueint;      
    }
    else{
        return -1;
    }
}
int64 FInternetHandle::Write(const uint8* outBuffer, int64 bytesToWrite){
    return 0;
}
bool FInternetHandle::Seek(int64 newPosition){
    if(newPosition < 0)
        return false;
    pos = newPosition;
    return true;
}
bool FInternetHandle::SeekFromEnd(int64 newPosition){
    if(newPosition < -size)
        return false;
    pos = size + newPosition;
    return true;
}
bool FInternetHandle::Flush(){
    return true;
}
bool FInternetHandle::Close(){
    return true;
}
FInternetHandle::~FInternetHandle(){
    curl_easy_cleanup(curl);    
    curl = NULL;
}

int64 FInternetLoader::FileSize(const char* fileName){
    CURL* curl;
    CURLcode res;
    Memory chunk;
    curl = curl_easy_init();
    if(curl){
        FString url = FString(ip) + "/download?fileName=" + FString(fileName) + "&pos=0&length=0";
        DEBUG(url.GetStr());
        curl_easy_setopt(curl, CURLOPT_URL, url.GetStr().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, readFromData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

        res = curl_easy_perform(curl);
        long retCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retCode);

        if(res != 0)
            curl_easy_cleanup(curl);

        cJSON *json, *jsonSize;
        if(retCode == 200){
            json = cJSON_Parse((char*)chunk.memory);
            if(!json){
                DEBUG("Internet Read CJOSN failed");
                return -1;
            }
            jsonSize = cJSON_GetObjectItem(json, "size");
            if(!jsonSize){
                DEBUG("Get Internet Size, json parse failed");
                return -1;
            }
            return jsonSize -> valueint;
        }
        
    }
    return -1;
}

// If the file is Existing
bool FInternetLoader::FileExists(const char* fileName){
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl){
        Memory chunk;
        FString url = FString(ip) + "/download?fileName=" + FString(fileName) + "&pos=0&length=0";
        DEBUG(url.GetStr());
        curl_easy_setopt(curl, CURLOPT_URL, url.GetStr().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, readFromData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

        res = curl_easy_perform(curl);
        long retCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retCode);

        if(res != 0)
            curl_easy_cleanup(curl);
        
        cJSON *json, *jsonSize;
        if(retCode == 200){
            json = cJSON_Parse((char*)chunk.memory);
            if(!json){
                DEBUG("Internet Read CJOSN failed");
                return -1;
            }
            jsonSize = cJSON_GetObjectItem(json, "size");
            if(!jsonSize){
                DEBUG("Get Internet Size, json parse failed");
                return -1;
            }
            if(jsonSize -> valueint == 0)
                return false;
            else
                return true;
        }
        
    }
    return false;
}

bool FInternetLoader::FileDelete(const char* fileName){
    return false;
}

bool FInternetLoader::FileMove(const char* fileFrom, const char* fileTo){
    return false;
}

bool FInternetLoader::FileCopy(const char* fileFrom, const char* fileTo){
    return false;
}

/*
    * Return read handle of file
    * @param fileName  the name of file to read
    * @return          the read handle of file
    */
FHandle* FInternetLoader::OpenRead(const char* fileName){
    // int64 fSize = FileSize(fileName);
    // if(fSize == 0)
    //     return nullptr;
    FHandle* retHandle = new FInternetHandle(ip, fileName, 0, 0);
    return retHandle;
}

/*
    * Return write handle of file
    * @param fileName  the name of file to write
    * @param append    if append to the end of file
    * @return          the write handle of file
    */
FHandle* FInternetLoader::OpenWrite(const char* fileName, bool append){
    return nullptr;
}

// Directory operation
bool FInternetLoader::DirectoryExist(const char* directoryName){
    return false;
}

bool FInternetLoader::DirectoryCreate(const char* directoryName){
    return false;
}

bool FInternetLoader::DirectoryDelete(const char* directoryName){
    return false;
}

/*
    * Find files with extension in the given directory
    * @param foundFiles    array to save the files found
    * @param directory     path to find files
    * @param extension     file filter
    */
void FInternetLoader::FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension){}

    /*
    * Find files with extension in the given directory recursively
    * @param foundFiles    array to save the files found
    * @param directory     path to find files
    * @param extension     file filter
    */
void FInternetLoader::FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension){}

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
    FHandle* handle = physicalLoader -> OpenWrite(targetPath.GetStr().c_str() , false);
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
    FLinuxLoader* loader = FLinuxLoader::GetFLinuxLoader();
    FHandle* handle = loader -> OpenWrite("/home/wall/data/test.pak", false);
    FWriteArchive* wArchive = new FWriteArchive(handle, "/home/wall/data/test.pak", 0);
    PakFile pakFile;

   char a[4] = {'q', 'b', 'c', 'd'};
   char c[3] = {'1', '2', '3'};
   pakFile.Write(handle, "123", (uint8*)a, 4, "");
   pakFile.Write(handle, "456", (uint8*)c, 3, "");

   pakFile.Serialize(*wArchive);

   wArchive -> Flush();

    FHandle* readHandle = loader -> OpenRead("/home/wall/data/test.pak");
    FReadArchive* rArchive = new FReadArchive(readHandle, "/home/wall/data/test.pak", readHandle -> Size());

    PakFile pakFile1;
    pakFile1.initialize(*rArchive);
    pakFile1.Write(handle, fileName, data, byteToWrite, "");
    handle -> Flush();

    PakEntry pakEntry;
    pakFile1.FindFile(fileName, pakEntry);
    uint8 p[byteToWrite];
    int readSize = pakFile1.Read(readHandle, pakEntry, p);
    DEBUG("read size : " << readSize);
    pakFile1.Print();
    pakFile1.Serialize(*wArchive);
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

    // if(pos == size){
    //     WriteToPak(data, size, fileName.GetStr().c_str());
    //     delete[] data;
    //     data = NULL;
    // }
    return btyeToWrite;
}

Resume::~Resume(){
    if(pos != size){
        WriteToDisk();
    }
    if(data != NULL){
        delete[] data;
        data = NULL;
    }

}