#ifndef FILEARRAYH
#define FILEARRAYH
#include "FileLoaderTypes.h"
#include <vector>
#include <iostream>


template <class ArrayType>
class FArray{
    std::vector<ArrayType> array;

public:
    FArray() {}

    void PushBack(ArrayType& InData) { array.push_back(InData); }
    void PushBack(ArrayType&& InData) { array.push_back(InData); }

    int32 Size() { return array.size(); }

    ArrayType& operator[](int32 pos){ 
        if(pos >= 0 && pos < Size())
            return array[pos];
        else{
            std::cout << "the pos is out range of FArray!" << std::endl;
            ArrayType* temp = NULL;
            return *temp;
        }
    }

    bool Erase(int32 pos){
        if(pos < 0 || pos >= Size())
            return false;
        else{
            auto it = array.begin();
            it += pos;
            array.erase(it);
            return true;
        }
    }
};



#endif