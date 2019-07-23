#include "FileString.h"
#include "FileLoaderTypes.h"
#include "FileLoader.h"
#include "FileArchive.h"
#include "FileArray.h"
#include "FilePakLoader.h"
#include "Hash.h"
#include "FileInternetLoader.h"
#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <zlib.h>
using namespace std;


int main(){
   char q[500] = {'v', 'g', 'r', 'e', 'w'};
   for(int j =0 ; j < 5; j++){
      for(int i = 0; i < 100; i ++){
         q[i+ 100*j] = 20+i;
      }
   }

   FPhysicalLoader* loader = FLinuxLoader::GetFWindowsLoader();
   FHandle* handle = loader -> OpenWrite("/home/wall/data/test.pak", false);
   FWriteArchive* wArchive = new FWriteArchive(handle, "/home/wall/data/test.pak", 0);
   
   Resume resume(loader, "wall", 500, 1);
   uint64 pos = resume.GetPos();
   DEBUG("write start with pos : " << pos);
   if(pos < resume.GetSize()){
      pos += resume.Write((uint8*)q + pos, 100);
   }
   /**
    * Resume test
    */


   /**
    * PakFile Write test
    */
   // PakFile pakFile;

   // char a[4] = {'q', 'b', 'c', 'd'};
   // char c[3] = {'1', '2', '3'};
   // pakFile.SetVersion(1);
   // pakFile.Write(handle, "123", (uint8*)a, 4);
   // pakFile.Write(handle, "456", (uint8*)c, 3);

   // pakFile.Serialize(*wArchive);

   // wArchive -> Flush();

   // FHandle* readHandle = loader -> OpenRead("/home/wall/data/test.pak");
   // FReadArchive* rArchive = new FReadArchive(readHandle, "/home/wall/data/test.pak", readHandle -> Size());

   // /** 
   //  * pakFile Read test
   //  */
   // PakFile pakFile1;
   // pakFile1.initialize(*rArchive);
   // PakEntry pakEntry;
   // pakFile1.Remove("123");
   // char q[10000] = {'v', 'g', 'r', 'e', 'w'};
   // for(int j =0 ; j < 100; j++){
   // for(int i = 0; i < 100; i ++){
   //    q[i+ 100*j] = 20+i;
   // }
   // }
   // pakFile1.Write(handle, "098", (uint8*)q, 10000);
   // handle -> Flush();
   // pakFile1.FindFile("098", pakEntry);
   // uint8 p[10000];
   // int readSize = pakFile1.Read(readHandle, pakEntry, p);
   // DEBUG("read size : " << readSize);
   // pakFile1.Print();
   // for(int i = 0; i < 10000; i++){
   //    cout << *(p+i);
   // }



   /**
    * PakFile base read test
    */
   // PakFile p2;
   // p2.Serialize(*rArchive);
   //  cout << "now indexSize is " << p2.info.GetIndexSize() << endl; 
   //  cout << "now index Offset is " << p2.info.GetIndexOffset() << endl;
   //  cout << p2.files.Size() << endl;
   //  cout << "first file is " << p2.files[0].blockList[0].GetStart() << " -> " << p2.files[0].blockList[0].GetEnd() << endl; 
   //  cout << "second file is " << p2.files[1].blockList[0].GetStart() << " -> " << p2.files[1].blockList[0].GetEnd() << endl;
   // char b[7];
   // p2.Read(handle, "123", (uint8*)b, 4);
   // if(!readHandle -> Seek(12)){
   //    DEBUG("Seek failed");
   // }
   // int copySize = readHandle -> Read((uint8*)b, 4);
   // DEBUG(copySize);
   // for(int i = 0; i < 4; i++){
   //    cout << b[i];
   // }



   /*
    * info, PakEntry test
    */
   // PakFile pakFile;
   // FileBlock fb1;
   // PakEntry pe1;
   // fb1.start = 22;
   // pe1.blockList.PushBack(fb1);
   // pe1.fBSize = 1;
   // pe1.hashTwo = 44;
   // pakFile.files.PushBack(pe1);
   // DEBUG(pakFile.info.indexOffset);
   // pakFile.Serialize(*wArchive);
   // DEBUG(pakFile.info.indexSize);
   // DEBUG(pakFile.files[0].hashTwo);
   // DEBUG(pakFile.files[0].blockList[0].start);

   // PakFile pf2;
   // pf2.Serialize(*rArchive);
   // DEBUG(pf2.info.indexSize);
   // DEBUG(pf2.files[0].hashTwo);
   // DEBUG(pf2.files[0].blockList[0].start);





   /*
    * Windos loader test
    */
   // FWindowsLoader* p = FWindowsLoader::GetFWindowsLoader();
   // if(p){
   //    FHandle* handle = p -> OpenWrite("D:\\data\\123.pak", true);
   //    FWriteArchive* wArchive = new FWriteArchive(handle, "D:\\data\\123.pak", 0);
   //    PakFile pakFile;
   //    pakFile.Serialize(*wArchive);
   // }




   /**
    * FString test
    */
   // FString s  = "C:\\\\test\\asf.sts";
   // s.NormalizeFileName();
   // cout << s.GetStr() << endl;
   // cout << s.GetStr() << endl;
   // FString filaName = s.GetFileName();
   // FString pathName = s.GetPath();
   // cout << filaName.GetStr() << endl;
   // cout << pathName.GetStr() << endl;
   // FString s1 = "C:/test";
   // s1.NormallizePath();
   // cout << s1.GetStr() << endl;


    /**
     * zlib test
     */

    // int err;
    // uint8 compr[200], uncompr[200];    // big enough
    // uLong comprLen, uncomprLen;
    // const char* hello = "12345678901234567890123456789012345678901234567890";
    // uLong len = strlen(hello) + 1;
    // comprLen = sizeof(compr) / sizeof(compr[0]);
    // err = compress(compr, &comprLen, (const Bytef*)hello, len);
    // if (err != Z_OK) {
    //     cerr << "compess error: " << err << '\n';
    //     exit(1);
    // }
    // cout << "orignal size: " << len 
    //      << " , compressed size : " << comprLen << '\n';
    // strcpy((char*)uncompr, "garbage");
    // err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    // if (err != Z_OK) {
    //     cerr << "uncompess error: " << err << '\n';
    //     exit(1);
    // }
    // cout << "orignal size: " << len
    //      << " , uncompressed size : " << uncomprLen << '\n';
    // if (strcmp((char*)uncompr, hello)) {
    //     cerr << "BAD uncompress!!!\n";
    //     exit(1);
    // } else {
    //     cout << "uncompress() succeed: \n" << (char *)uncompr;
    // }



}


