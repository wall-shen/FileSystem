#ifndef FILELOADERTYPESH
#define FILELOADERTYPESH

// #define INTERNET_IP             "http://162.219.126.98:8001"
// #define INTERNET_IP "http://127.0.0.1:8001"
#define INTERNET_IP "http://10.240.149.34:8001"
#define PAKLOADER_DIRECTORY     "/home/wall/data/"
#define PAK_EXTENSION           ".pak"

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;
typedef signed char    int8;
typedef signed short   int16;
typedef signed int     int32;
typedef signed long    int64;


enum{
    DefaultBufferSize = 2048000,
};

enum{
    HttpRequestLength = 102400,  
};

enum{
    NativeFile   = 0x00,
    PkgFile      = 0x01,
    DownLoadFile = 0x02,
}FileType;

enum{
    ZipMethod = 0x0001,
}CompressMethod;
#endif