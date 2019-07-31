#ifndef FILESTRINGH
#define FILESTRINGH

#include <string>
#include "FileLoaderTypes.h"
#include <string.h>

class FString{
    std::string str;
public:
    FString(std::string InStr = ""){
        str = InStr;
    }

    FString(const char* Instring){
        str = Instring;
    }

    FString(const FString& InFString){
        str = InFString.str;
    }

    FString(FString&& InFString){
        str = InFString.str;
    }

    char& operator[] (int32 pos){
        return str[pos];
    }

    FString& operator= (FString& InStr){
        str = InStr.str;
        return *this;
    }

    FString& operator= (std::string InStr){
        str = InStr;
        return *this;
    }

    FString& operator= (const char* InStr){
        str = InStr;
        return *this;
    }

    FString& operator=(FString&& InStr){
        str = InStr.str;
        return *this;
    }

    FString operator+ (const FString& InStr){
        return FString(str + InStr.str); 
    }

    FString operator+ (const std::string InStr){
        return FString(str + InStr);
    }

    FString operator+ (const char* InStr){
        return FString(str + InStr);
    }

    friend FString operator+ (const char* InChar, FString InFString){
        return FString(InChar + InFString.GetStr());
    }

    int32 operator== (FString& InStr){
        return str == InStr.str;
    }

    int32 operator== (const char* InStr){
        return str == InStr;
    }

    friend bool operator< (const FString& str1, const FString& str2){
        return str1.str < str2.str;
    }

    std::string& GetStr(){ return str;}

    int64 GetSize() { return str.size(); }


    bool EndWith(FString& InStr){
        return str.rfind(InStr.str) == (str.size() - InStr.str.size()) ? true : false;
    }

    bool EndWith(const char* InStr){
        return str.rfind(InStr) == (str.size() - strlen(InStr)) ? true : false;
    }

    bool StartWith(FString& InStr){
        return str.find(InStr.str) == 0 ? true : false;
    }

    bool StartWith(const char* InStr){
        return str.find(InStr) == 0 ? true : false;
    }

    // path end with '/'
    void NormallizePath(){
        if(str[str.size() -1] != '/')
            str.append(1, '/');
    }

    void NormalizeFileName(){
        for(int i = 0; i < str.size() ; i++){
            if(str[i] == '\\')
                if(str[i+1] == '\\'){
                    str.replace(i, 2, "/");
                }
                else{
                    str.replace(i, 1, "/");
                }
        }
    }
    
    FString GetFileName(){
        int pos = str.rfind('/');
        FString s = str.substr(pos + 1);
        return s;
    }

    FString GetNextStr(FString cutStr){
        int pos = str.find(cutStr.GetStr());
        FString s = str.substr(pos + cutStr.GetSize());
        return s;
    }

    // path end with '/' 
    FString GetPath(){
        int pos = str.rfind('/');
        FString s = str.substr(0, pos + 1);
        return s;
    }

    int32 GetSerializedSize(){
        return sizeof(int32) + str.size() + 1;
    }

};

#endif