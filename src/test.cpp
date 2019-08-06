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
#include <string.h>
#include <zlib.h>
#include <curl/curl.h>
#include <cstdlib>
#include <time.h>
#include <sys/time.h>
#include "FileInternetLoader.h"
#include "FileManager.h"
#include "Threadpool.h"
using namespace std;

int main(){
   /**
    * write direct demo
    */
   // FLinuxLoader* load = FLinuxLoader::GetFLinuxLoader();
   // timeval timeStart, timeEnd;
   // gettimeofday(&timeStart, NULL);

   // FPakLoader* pakLoader = FPakLoader::GetFPakLoader();
   // pakLoader -> CreatePak("/home/wall/data/test.pak");
   // FManager manager;
   // manager.Update();
   // gettimeofday(&timeEnd, NULL);
   // double costTime = (timeEnd.tv_sec-timeStart.tv_sec) + (timeEnd.tv_usec-timeStart.tv_usec)/1000000.0;

   // // for(int i = 0; i < pakLoader -> pakFiles.Size(); i++){
   // //    pakLoader -> pakFiles[i].Print();
   // // }
   // DEBUG("Download cost time " << costTime << "s");

   /**
    * write test demo
    */
   FLinuxLoader* load = FLinuxLoader::GetFLinuxLoader();
   timeval timeStart, timeEnd;
   gettimeofday(&timeStart, NULL);

   FPakLoader* pakLoader = FPakLoader::GetFPakLoader();
   pakLoader -> CreatePak("/home/wall/data/test.pak");
   FManager manager;
   manager.Update();
   gettimeofday(&timeEnd, NULL);
   double costTime1 = (timeEnd.tv_sec-timeStart.tv_sec) + (timeEnd.tv_usec-timeStart.tv_usec)/1000000.0;

   gettimeofday(&timeStart, NULL);
   Combine();

   gettimeofday(&timeEnd, NULL);
   double costTime = (timeEnd.tv_sec-timeStart.tv_sec) + (timeEnd.tv_usec-timeStart.tv_usec)/1000000.0;
   DEBUG("Download cost time " << costTime1 << "s");
   DEBUG("Write cost time " << costTime << "s");

   FHandle* rHandle = pakLoader -> OpenRead("399");
   int64 readSize= pakLoader-> FileSize("399");
   cout << readSize << endl;
   char p[readSize];
    rHandle -> Read((uint8*)p, readSize);
    for(int i = readSize - 500 ; i < readSize; i++){
       cout << p[i];
    }

      /**
    * MD5 test
    */
   // uint8 p[] = "this is test";
   // uint8 result[16];
   // MD5_CTX md5;
   // MD5Init(&md5);
   // MD5Update(&md5, p, strlen((char*)p));
   // MD5Final(&md5, result);
   // for (int i = 0; i<16; i++)
	// {
	// 	printf("%02x", result[i]);  
	// }


   // FHandle* rHandle = pakLoader -> OpenRead("2");
   // int64 readSize= pakLoader-> FileSize("2");
   // cout << readSize << endl;
   // char p[readSize];
   //  rHandle -> Read((uint8*)p, readSize);
   //  for(int i = readSize - 500 ; i < readSize; i++){
   //     cout << p[i];
   //  }

   // FInternetLoader intLoader;
   // FHandle* handle = intLoader.OpenRead("updateMessage");
   // char p [72019];
   // int size = 72019;
   
   // int writeSize = 0;
   // while(writeSize < 72092){
   //    int copySize = handle -> Read((uint8*)(p+writeSize), 10000);
   //    if(copySize <= 0){
   //       DEBUG("wirte over " << copySize);
   //       break;
   //    }
   //    writeSize += copySize;
   // }

/**
 * create test data
 */
   // srand((unsigned)time(NULL));
   // FHandle* dataHandle = load -> OpenWrite("/root/data/updateData", false);
   // FHandle* MessageHandle = load -> OpenWrite("/root/data/updateMessage", false);
   // cJSON *root;
   // root = cJSON_CreateArray();
   // int64 offset = 0;
   // for(int i = 0; i < 500; i++){
   //    int64 filesize = rand() % (512 * 1024) + 1024 * 1024;
   //    char data[filesize];
   //    for(int j = 0; j < filesize; j++){
   //       data[j] = '0' + (j % 10);
   //    }

   //    uint8 result[16];
   //    MD5_CTX md5;
   //    MD5Init(&md5);
   //    MD5Update(&md5, (uint8*)data, filesize);
   //    MD5Final(&md5, result);
   //    char re[17];
   //    memcpy((char*)re, (char*)result, 16);
   //    re[16] = '\0';
   //    FString md5Str = re;
   //    DEBUG(md5Str.GetStr());
   //    FString s = to_string(i);
   //    cJSON_AddItemToArray(root, cJSON_CreateString(s.GetStr().c_str()));
   //    cJSON_AddItemToArray(root, cJSON_CreateString("updateData"));
   //    cJSON_AddItemToArray(root, cJSON_CreateString(md5Str.GetStr().c_str()));
   //    cJSON_AddItemToArray(root, cJSON_CreateNumber(offset));
   //    cJSON_AddItemToArray(root, cJSON_CreateNumber(filesize));
   //    cJSON_AddItemToArray(root, cJSON_CreateNumber(0));

   //    dataHandle -> Write((uint8*)data, filesize);

   //    offset += filesize;
   // }

   // char * p = cJSON_PrintUnformatted(root);
   // DEBUG(p);
   // int64 size = strlen(p);
   // MessageHandle -> Write((uint8*)p, size);

   // char* p  = "[\"1\", "",\"md4\", 53, 123]";
   // cJSON* root;
   // root = cJSON_Parse(p);
   // int64 num = cJSON_GetArraySize(root);
   // cJSON* item;
   // num /= 4;
   // for(int i = 0; i < num; i++){
   //    item = cJSON_GetArrayItem(root , i);
   //    printf("item type is %d\n",item->type);
   //  printf("%s\n",item->valuestring);
   //  item = cJSON_GetArrayItem(root , i+1);
   //    printf("item type is %d\n",item->type);
   //  printf("%s\n",item->valuestring);
   //  item = cJSON_GetArrayItem(root , i+2);
   //    printf("item type is %d\n",item->type);
   //  printf("%d\n",item->valueint);
   //  item = cJSON_GetArrayItem(root , i+3);
   //    printf("item type is %d\n",item->type);
   //  printf("%d\n",item->valueint);
   // }


   // char q[500];
   // for(int j =0 ; j < 50; j++){
   //    for(int i = 0; i < 10; i ++){
   //       q[i+ 10*j] = '0'+i;
   //    }
   // }

   // cout << q << endl;
   //  FPakLoader* pakLoader = FPakLoader::GetFPakLoader();
   //  pakLoader -> CreatePak("/home/wall/data/test.pak");
   //  cout << pakLoader -> pakFiles.Size() << endl;
   //  pakLoader -> pakFiles[0].Remove("500demo");
   //  cout << "500demo over" << endl;
   //  pakLoader -> pakFiles[0].CreateFile("temp", 190, 190, 0);
   //  pakLoader -> pakFiles[0].Remove("temp");
   //  pakLoader -> pakFiles[0].CreateFile("500demo", 500, 500, 0);
   //  FHandle* whandle = pakLoader -> OpenWrite("500demo", false);
   //  whandle -> Write((uint8*)q, 500);
   //  for(int i = 0; i < pakLoader -> pakFiles.Size(); i++){
   //      pakLoader -> pakFiles[i].Print();
   //  }
   // char p[100];
   //  FHandle* rHandle = pakLoader -> OpenRead("1");

   //  rHandle -> Read((uint8*)p, 100);
   //  for(int i = 0 ; i < 100; i++){
   //     cout << p[i];
   //  }
   //  rHandle = pakLoader -> OpenRead("2");

   //  rHandle -> Read((uint8*)p, 100);
   //  cout << p ;
   //  rHandle = pakLoader -> OpenRead("3");

   //  rHandle -> Read((uint8*)p, 100);

   // rHandle = pakLoader -> OpenRead("4");

   //  rHandle -> Read((uint8*)p, 100);
   //  cout << p;
   //  rHandle = pakLoader -> OpenRead("5");

   //  rHandle -> Read((uint8*)p, 100);
   //  cout << p ;

/**
 * compress resume test
 */
    // FInternetLoader intLoader;
    // FHandle* interHandle = intLoader.OpenRead("500demo");

    // FManager manager;
    // manager.WriteToPak(interHandle, "/home/wall/data/test.pak", "500demo", 500, 1);


//     FLinuxLoader* loader = FLinuxLoader::GetFLinuxLoader();
//    FHandle* readHandle = loader -> OpenRead("/home/wall/data/test.pak");
//    FReadArchive* rArchive = new FReadArchive(readHandle, "/home/wall/data/test.pak", readHandle -> Size());
//    PakFile pakFile1;

//     pakFile1.initialize(*rArchive);
//     PakEntry pakEntry;
//     pakFile1.FindFile("500demo", pakEntry);
//     uint8 p[500];
//     int readSize = pakFile1.Read(readHandle, pakEntry, p);
//     cout << p;





/**
 * internet Test
 */

    // FInternetLoader intLoader;
    // FHandle* interHandle = intLoader.OpenRead("demo");
    // uint8 q[4];
    
 
//    FHandle* handle = loader -> OpenWrite("/home/wall/data/500demo", false);
//     handle -> Write((uint8*)q, 500);
 
/**
* Resume test
*/

//    Resume resume(loader, "wall", 12, 1);
//    uint64 pos = resume.GetPos();
//    DEBUG("write start with pos : " << pos);
//    if(pos < resume.GetSize()){
//        interHandle -> Seek(pos);
//        interHandle -> Read(q, 4);
//       pos += resume.Write((uint8*)q, 4);
//    }


//    /**
//     * PakFile Write test
//     */
//     FPhysicalLoader* loader = FLinuxLoader::GetFLinuxLoader();
//     FHandle* handle = loader -> OpenWrite("/home/wall/data/test.pak", false);
//     FWriteArchive* wArchive = new FWriteArchive(handle, "/home/wall/data/test.pak", 0);

//    PakFile pakFile;

//    char a[4] = {'q', 'b', 'c', 'd'};
//    char c[3] = {'1', '2', '3'};
//    pakFile.SetVersion(1);
//    pakFile.SetMountPoint("/test");
//    pakFile.Write(handle, "123", (uint8*)a, 4);
//    pakFile.Write(handle, "456", (uint8*)c, 3);

//    pakFile.Serialize(*wArchive);
//    pakFile.Print();

//    wArchive -> Flush();


//    /** 
//     * pakFile Read test
//     */
 
//    pakFile1.Remove("123");
//    char q[10000] = {'v', 'g', 'r', 'e', 'w'};
//    for(int j =0 ; j < 100; j++){
//    for(int i = 0; i < 100; i ++){
//       q[i+ 100*j] = 20+i;
//    }
//    }
//    pakFile1.Write(handle, "098", (uint8*)q, 10000);
//    handle -> Flush();

//    pakFile1.Serialize(*wArchive);
//    DEBUG("read size : " << readSize);
//    pakFile1.Print();
//    for(int i = 0; i < 10000; i++){
//       cout << *(p+i);
//    }



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


