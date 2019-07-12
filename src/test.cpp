#include "FileString.h"
#include "FileLoaderTypes.h"
#include "FileLoader.h"
#include "FileArchive.h"
#include "FileArray.h"
#include "FilePakLoader.h"
#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
using namespace std;

int main(){

   FWindowsLoader* loader = FWindowsLoader::GetFWindowsLoader();
   FArray<FString> f;
   FHandle* handle = loader -> OpenWrite("D:\\data\\qwe", true);
   FWriteArchive* wArchive = new FWriteArchive(handle, "12", 0);
   uint8 buffer[10] = {'q','r','a','a','a','z','z','q','a','b'};
   wArchive -> Serialize(buffer, 10);
   wArchive -> Flush();
   FHandle* readHandle = loader -> OpenRead("D:\\data\\qwe");
   FReadArchive* rArchive = new FReadArchive(readHandle, "12", readHandle -> Size());
   uint8 inBuffer[10];
   rArchive -> Serialize(inBuffer, 10);
   for(int i = 0; i < 7; i++){
      cout << inBuffer[i] << endl;
   }
   
   // FWindowsLoader* p = FWindowsLoader::GetFWindowsLoader();
   // if(p){
   //    FHandle* handle = p -> OpenWrite("D:\\data\\123.pak", true);
   //    FWriteArchive* wArchive = new FWriteArchive(handle, "D:\\data\\123.pak", 0);
   //    PakFile pakFile;
   //    pakFile.Serialize(*wArchive);
   // }

}


