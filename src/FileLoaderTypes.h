#ifndef FILELOADERTYPESH
#define FILELOADERTYPESH
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;
typedef signed char    int8;
typedef signed short   int16;
typedef signed int     int32;
typedef signed long    int64;

enum{
    NativeFile   = 0x00,
    PkgFile      = 0x01,
    DownLoadFile = 0x02,
}FileType;

enum{
    ZipMethod = 0x0001,
}CompressMethod;

#endif