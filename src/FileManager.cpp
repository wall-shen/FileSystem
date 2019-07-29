#include "FileManager.h"


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

/*
    * Return read handle of file
    * @param fileName  the name of file to read
    * @return          the read handle of file
    */
FHandle* FManager::OpenRead(const char* fileName){
    return nullptr;
}

/*
    * Return write handle of file
    * @param fileName  the name of file to write
    * @param append    if append to the end of file
    * @return          the write handle of file
    */
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

/*
    * Find files with extension in the given directory
    * @param foundFiles    array to save the files found
    * @param directory     path to find files
    * @param extension     file filter
    */
void FManager::FindFiles(FArray<FString>& foundFiles, const char* directory, const char* extension){}

    /*
    * Find files with extension in the given directory recursively
    * @param foundFiles    array to save the files found
    * @param directory     path to find files
    * @param extension     file filter
    */
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
                if(resume.GetPos() % 200 == 0){
                    DEBUG("continue?");
                    int flag;
                    std::cin >> flag;
                    if(flag == 0)
                        return resume.GetPos();
                }
            }
            // pakLoader -> Write(InpakName, fileName, resume.GetData(), size);
        }
    }
    return 0;
}

FManager::~FManager(){}