#include "FileManager.h"
#include "Threadpool.h"

#define DefaultThreadNum 16

#define UpdateMessageFile "updateMessage"

FManager::FManager(){}

bool FManager::AddLoader(int16 loaderIndex, const char* loaderPara){
    return true;
}

bool FManager::ListClear(){
    return true;
}

int64 FManager::FileSize(const char* fileName){
    return -1;
}

// If the file is Existing
bool FManager::FileExists(const char* fileName){
    return false;
}

bool FManager::FileDelete(const char* fileName){
    return false;
}

bool FManager::FileMove(const char* fileFrom, const char* fileTo){
    return false;
}

bool FManager::FileCopy(const char* fileFrom, const char* fileTo){
    return false;
}

FHandle* FManager::OpenRead(const char* fileName){
    return nullptr;
}


FHandle* FManager::OpenWrite(const char* fileName, bool append){
    return nullptr;
}

// Directory operation
bool FManager::DirectoryExist(const char* directoryName){
    return false;
}

bool FManager::DirectoryCreate(const char* directoryName){
    return false;
}

bool FManager::DirectoryDelete(const char* directoryName){
    return false;
}

void FManager::FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension){}

void FManager::FindFilesRecursively(FArray<FString>& foundFiles, const char* directory, const char* extension){}

int64 FManager::WriteToPak(FHandle* handle, const char* InpakName, const char* fileName, int64 size, int32 version){
    if(!handle || !InpakName || !fileName || size < 0 || version < 0)
        return -1;

    FString pakName = InpakName;
    FPakLoader* pakLoader = FPakLoader::GetFPakLoader();
    for(int i = 0; i < pakLoader -> pakFiles.Size(); i++){
        if(pakLoader -> pakFiles[i].GetFileName() == pakName){
            Resume resume(fileName, size, version);
            DEBUG("write start with pos : " << resume.GetPos());
            uint8 q[100];
            while(resume.GetPos() < resume.GetSize()){
                handle -> Seek(resume.GetPos());
                int64 readSize = handle -> Read(q, 100);
                resume.Write(q, readSize);
                // if(resume.GetPos() % 200 == 0){
                //     DEBUG("continue?");
                //     int flag;
                //     std::cin >> flag;
                //     if(flag == 0)
                //         return resume.GetPos();
                // }
            }
            pakLoader -> Write(InpakName, fileName, resume.GetData(), size, "");
        }
    }
    return 0;
}


class DownloadTask{
    FString mOutFileName;
    FString mInFileName;
    FHandle* writeHandle;
    FHandle* readHandle;
    int64 size;
    int32 bufferSize;
    uint8* buffer;
    int64 mOffset;
    int64 mPos;
public:
    DownloadTask(const char* outFileName, int64 pos, const char* inFileName, int64 offset, int64 readSize, int32 InbufferSize = DefaultBufferSize)
        : size(readSize)
        , mOutFileName(outFileName)
        , mInFileName(inFileName)
        , writeHandle(nullptr)
        , readHandle(nullptr)
        , buffer(nullptr)
        , mOffset(offset)
        , mPos(pos)
        , bufferSize(InbufferSize)
    {}

    DownloadTask(DownloadTask&& temp){
        writeHandle = temp.writeHandle;
        readHandle = temp.readHandle;
        size = temp.size;
        mOutFileName = temp.mOutFileName;
        mInFileName = temp.mInFileName;
        mOffset = temp.mOffset;
        mPos = temp.mPos;
        bufferSize = temp.bufferSize;
        buffer = temp.buffer;
        temp.writeHandle = nullptr;
        temp.readHandle = nullptr;
        temp.buffer = nullptr;
    }

    void operator()();

    ~DownloadTask(){
        if(writeHandle){
            delete writeHandle;
            writeHandle = nullptr;
        }
        if(readHandle){
            delete readHandle;
            readHandle = nullptr;
        }
        if(buffer){
            delete[] buffer;
            buffer = nullptr;
        }
    }
};

void DownloadTask::operator()(){

    /**
     * Normal version
     */
    // if(buffer == nullptr){
    //     buffer = new uint8[bufferSize];
    // }
    // if(writeHandle == nullptr){
    //     FPakLoader* pakLoader = FPakLoader::GetFPakLoader();
    //     writeHandle  = pakLoader -> OpenWrite(mOutFileName.GetStr().c_str());
    //     if(writeHandle)
    //         writeHandle -> Seek(mPos);
    // }
    // if(readHandle == nullptr){
    //     FInternetLoader intLoader;
    //     readHandle = intLoader.OpenRead(mInFileName.GetStr().c_str());
    //     if(readHandle)
    //         readHandle -> Seek(mOffset);
    // }
    // int32 copySize = 0;
    // int64 leftToWrite = size;
    // while(leftToWrite > 0){
    //     if(bufferSize - copySize < HttpRequestLength){
    //         int64 writeSize = writeHandle -> Write(buffer , copySize);
    //         if(writeSize != copySize){
    //             DEBUG("DownladTask operator() write Failed");
    //             return;
    //         }
    //         copySize = 0;
    //     }

    //     int64 readSize = 0;
    //     if(leftToWrite < HttpRequestLength){
    //         readSize = readHandle -> Read(buffer + copySize, leftToWrite);
    //         if(readSize != leftToWrite){
    //             DEBUG("DownladTask operator() read leftToWrite Failed");
    //             return;
    //         }
            
    //     }
    //     else{
    //         readSize = readHandle -> Read(buffer + copySize , HttpRequestLength);
    //         if(readSize != HttpRequestLength){
    //             DEBUG("DownladTask operator() read HttpRequestLength Failed");
    //             return;
    //         }
    //     }
    //     copySize += readSize;
    //     leftToWrite -= readSize;
    // }
    // if(copySize != 0){
    //     int64 writeSize = writeHandle -> Write(buffer , copySize);
    //         if(writeSize != copySize){
    //             DEBUG("DownladTask operator() write Failed");
    //             return;
    //         }
    //     copySize = 0;
    // }




    /**
     *  download and combine version
     */
    FString DefalultDir = "/home/wall/data/temp/";
    if(buffer == nullptr){
        buffer = new uint8[bufferSize];
    }
    if(writeHandle == nullptr){
        FLinuxLoader* phyLoader = FLinuxLoader::GetFLinuxLoader();
        mOutFileName = DefalultDir + mOutFileName + ".downloading";
        writeHandle  = phyLoader -> OpenWrite(mOutFileName.GetStr().c_str(), false);
        if(!writeHandle)
            return;
    }
    if(readHandle == nullptr){
        FInternetLoader intLoader;
        readHandle = intLoader.OpenRead(mInFileName.GetStr().c_str());
        if(readHandle)
            readHandle -> Seek(mOffset);
        else
            return;
    }
    int32 copySize = 0;
    int64 leftToWrite = size;
    while(leftToWrite > 0){
        if(bufferSize - copySize < HttpRequestLength){
            int64 writeSize = writeHandle -> Write(buffer , copySize);
            if(writeSize != copySize){
                DEBUG("DownladTask operator() write Failed");
                return;
            }
            copySize = 0;
        }

        int64 readSize = 0;
        if(leftToWrite < HttpRequestLength){
            readSize = readHandle -> Read(buffer + copySize, leftToWrite);
            if(readSize != leftToWrite){
                DEBUG("DownladTask operator() read leftToWrite Failed");
                return;
            }          
        }
        else{
            readSize = readHandle -> Read(buffer + copySize , HttpRequestLength);
            if(readSize != HttpRequestLength){
                DEBUG("DownladTask operator() read HttpRequestLength Failed");
                return;
            }
        }
        copySize += readSize;
        leftToWrite -= readSize;
    }
    if(copySize != 0){
        int64 writeSize = writeHandle -> Write(buffer , copySize);
            if(writeSize != copySize){
                DEBUG("DownladTask operator() write Failed");
                return;
            }
        copySize = 0;
    }
}

class TaskInfo{
public:
    FString mOutFileName;
    FString mInFileName;
    // outFile pos
    int64 mPos;

    // inFile offset
    int64 mOffset;
    int64 mSize;

    TaskInfo(FString outFileName, FString inFileName, int64 pos, int64 offset, int64 size)
        : mOutFileName(outFileName)
        , mInFileName(inFileName)
        , mPos(pos)
        , mOffset(offset)
        , mSize(size)
        {}
};


typedef FArray<TaskInfo> TaskList;
void GetTask(TaskList& list){
    FPakLoader* pakLoader = FPakLoader::GetFPakLoader();
    FInternetLoader intLoader;
    int64 messageSize = intLoader.FileSize(UpdateMessageFile);
    FHandle* intHandle = intLoader.OpenRead(UpdateMessageFile);
    if(!intHandle || messageSize == -1){
        DEBUG("GetTask request " << UpdateMessageFile << "failed");
        return;
    }

    uint8 josnMessage[messageSize];
    int64 readSize = intHandle ->Read(josnMessage, messageSize);
    if(readSize != messageSize){
        DEBUG("GetTask read from " << UpdateMessageFile << " size : " << messageSize << "failed");
        return;
    }

    cJSON* JsonArray, *ArrayItem;
    JsonArray = cJSON_Parse((char*)josnMessage);
    if(!JsonArray){
        DEBUG("GetTask json_Parse failed");
        return;
    }

    int32 arrSize = cJSON_GetArraySize(JsonArray);

    for(int i = 0; i < arrSize;){
        ArrayItem = cJSON_GetArrayItem(JsonArray, i++);
        FString fileName = ArrayItem -> valuestring;
        ArrayItem = cJSON_GetArrayItem(JsonArray, i++);
        FString serverFileName = ArrayItem -> valuestring;
        ArrayItem = cJSON_GetArrayItem(JsonArray, i++);
        FString md5 = ArrayItem -> valuestring;
        ArrayItem = cJSON_GetArrayItem(JsonArray, i++);
        int64 offset = ArrayItem -> valueint;
        ArrayItem = cJSON_GetArrayItem(JsonArray, i++);
        int64 size = ArrayItem -> valueint;
        ArrayItem = cJSON_GetArrayItem(JsonArray, i++);
        int64 compressMethod = ArrayItem -> valueint;

        int64 pos = pakLoader -> Compare(fileName.GetStr().c_str(), md5, size);
        if(pos == -1){
            pakLoader -> CreateEntry(fileName.GetStr().c_str(), size, size, compressMethod, md5);
            list.PushBack(TaskInfo(fileName, serverFileName, 0, offset, size));
        }
        else if(pos != size){
            list.PushBack(TaskInfo(fileName, serverFileName, pos, offset + pos, size - pos));
        }
    }
    if(JsonArray)
        cJSON_Delete(JsonArray);

}

void FManager::Update(){
    ThreadPool pool(16);
    TaskList tasklist;
    GetTask(tasklist);
    for(int i = 0; i < tasklist.Size(); i++){
        TaskInfo& info = tasklist[i];
        pool.enqueue(DownloadTask{info.mOutFileName.GetStr().c_str(), info.mPos, info.mInFileName.GetStr().c_str(), info.mOffset, info.mSize});
        // DownloadTask task{info.mOutFileName.GetStr().c_str(), info.mPos, info.mInFileName.GetStr().c_str(), info.mOffset, info.mSize};
        // task();
    }
}
 
FManager::~FManager(){}