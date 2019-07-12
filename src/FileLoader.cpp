#include "FileLoader.h"

FWindowsLoader* FWindowsLoader::singleFWindowsLoader = nullptr;

FWindowsLoader* FWindowsLoader::GetFWindowsLoader(){
    if(singleFWindowsLoader == nullptr){
        static FWindowsLoader staticWinsdowsLoader;
        singleFWindowsLoader = &staticWinsdowsLoader;
    }
    return singleFWindowsLoader;
}
