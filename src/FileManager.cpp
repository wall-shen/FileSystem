#include "FileManager.h"
#include "Threadpool.h"

#define DefaultThreadNum 10

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
            pakLoader -> Write(InpakName, fileName, resume.GetData(), size);
        }
    }
    return 0;
}

class DownloadTask{
    FHandle* writeHandle;
    FHandle* readHandle;
    int64 size;
    int32 bufferSize;
    uint8* buffer;
public:
    DownloadTask(const char* outFileName, int64 pos, const char* inFileName, int64 offset, int64 readSize, int32 InbufferSize = DefaultBufferSize)
        : size(readSize)
        , bufferSize(InbufferSize)
    {
        buffer = new uint8[bufferSize];
        FPakLoader* pakLoader = FPakLoader::GetFPakLoader();
        writeHandle  = pakLoader -> OpenWrite(outFileName);
        if(writeHandle)
            writeHandle -> Seek(pos);
        FInternetLoader intLoader;
        readHandle = intLoader.OpenRead(inFileName);
        if(readHandle)
            readHandle -> Seek(offset);
    }

    DownloadTask(DownloadTask&& temp){
        writeHandle = temp.writeHandle;
        readHandle = temp.readHandle;
        size = temp.size;
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

    int32 arrSize = cJSON_GetArraySize(JsonArray) / 6;

    for(int i = 0; i < arrSize; i++){
        ArrayItem = cJSON_GetArrayItem(JsonArray, i);
        FString fileName = ArrayItem -> valuestring;
        ArrayItem = cJSON_GetArrayItem(JsonArray, i + 1);
        FString serverFileName = ArrayItem -> valuestring;
        ArrayItem = cJSON_GetArrayItem(JsonArray, i + 2);
        FString md5 = ArrayItem -> valuestring;
        ArrayItem = cJSON_GetArrayItem(JsonArray, i + 3);
        int64 offset = ArrayItem -> valueint;
        ArrayItem = cJSON_GetArrayItem(JsonArray, i + 4);
        int64 size = ArrayItem -> valueint;
        ArrayItem = cJSON_GetArrayItem(JsonArray, i + 5);
        int64 compressMethod = ArrayItem -> valueint;

        int64 pos = pakLoader -> Compare(fileName.GetStr().c_str(), md5.GetStr().c_str(), size);
        if(pos == -1){
            pakLoader -> CreateEntry(fileName.GetStr().c_str(), size, size, compressMethod);
            list.PushBack(TaskInfo(fileName, serverFileName, 0, offset, size));
        }
        else if(pos != size){
            list.PushBack(TaskInfo(fileName, serverFileName, pos, offset + pos, size - pos));
        }
    }
    if(JsonArray)
        cJSON_Delete(JsonArray);
    if(ArrayItem)
        cJSON_Delete(ArrayItem);
    // FPakLoader* pakLoader = FPakLoader::GetFPakLoader();
    // pakLoader -> pakFiles[0].CreateFile("1", 100, 100, 0);
    // pakLoader -> pakFiles[0].CreateFile("2", 100, 100, 0);
    // pakLoader -> pakFiles[0].CreateFile("3", 100, 100, 0);
    // pakLoader -> pakFiles[0].CreateFile("4", 100, 100, 0);
    // pakLoader -> pakFiles[0].CreateFile("5", 100, 100, 0);


    // list.PushBack(TaskInfo("2", "500data.pak", 0, 100, 100));
    // list.PushBack(TaskInfo("3", "500data.pak", 0, 200, 100));
    // list.PushBack(TaskInfo("4", "500data.pak", 0, 300, 100));
    // list.PushBack(TaskInfo("5", "500data.pak", 0, 400, 100));
}

void FManager::Update(){
    ThreadPool pool(DefaultThreadNum);
    TaskList tasklist;
    GetTask(tasklist);
    for(int i = 0; i < tasklist.Size(); i++){
        TaskInfo& info = tasklist[i];
        // pool.enqueue(DownloadTask{info.mOutFileName, info.mPos, info.mInFileName, info.mOffset, info.mSize});
        DownloadTask task{info.mOutFileName.GetStr().c_str(), info.mPos, info.mInFileName.GetStr().c_str(), info.mOffset, info.mSize};
        task();
    }
}

FManager::~FManager(){}