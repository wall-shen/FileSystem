// #include<curl/curl.h>
// #include<stdio.h>
// #include<iostream>
// #include <string.h>
// #include"cJSON.h"
// using namespace std;

// cJSON *json, *jsonLength, *jsonSize, *jsonData;
// size_t readFromData(void* buffer, size_t size, size_t nmemeb, void* userPtr){
//   json = cJSON_Parse((char*)buffer);
//   if(!json){
//     return size * nmemeb;
//   }
//   char * src = cJSON_Print(json);
//   jsonLength = cJSON_GetObjectItem(json, "length");
//   jsonSize = cJSON_GetObjectItem(json, "size");
//   jsonData = cJSON_GetObjectItem(json, "data");
//   cout << src << endl;
//   cout << jsonData -> valuestring << endl;

//   cJSON_Delete(json);
//   return size * nmemeb; 
// }

// int main(){
//   CURL* curl;
//   CURLcode res;
//   char * data[200];
//   curl = curl_easy_init();
//   if(curl){
//     curl_easy_setopt(curl, CURLOPT_URL, "http://162.219.126.98:8001/download?fileName=demo&pos=12&length=4");
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, readFromData);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
//     //curl_easy_setopt(curl, CURLOPT_HEADERDATA, fp);

//     res = curl_easy_perform(curl);
//     long retCode = 0;
//     curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retCode);
//     if(retCode == 200){
//       if(!data)
// 	      return -1;
      
//     }
//     if(res != 0){
//       curl_easy_cleanup(curl);
//     }

//   }
// }
