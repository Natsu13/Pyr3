#include "CString.h"

int CString::sizeOfChar(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

void CString::empty() {
    if (size > 0)
        data[0] = '\0';
}

void CString::realoc() {
    data = (char*)malloc(size+1);
    if (data == nullptr)
        throw "Can't alloc enought memory";

    data[0] = '\0';
}

CString::CString() {
    size = 0;
    realoc();
}
CString::CString(const char* str) {
    if(str != NULL)
        size = sizeOfChar(str);

    realoc();

    if (str != NULL) {
        memcpy(data, str, size);
        data[size] = '\0';
    }
}

CString& CString::operator+(CString& second) {
    auto oldsize = size;
    size = size + second.size - 1;
    char* old = data;
    realoc();
    memcpy(data, old, oldsize - 1);
    memcpy(data + oldsize - 1, second.data, second.size);
    return *this;
}

CString& CString::operator+(const char* second) {
    auto oldsize = size;
    int sizeSecond = sizeOfChar(second);
    size = size + sizeSecond - 1;
    char* old = data;
    realoc();
    memcpy(data, old, oldsize - 1);
    memcpy(data + oldsize - 1, second, sizeSecond);    
    return *this;
}

CString& CString::operator+=(char second) {
    auto oldsize = size;
    size += 1;
    char* old = data;
    realoc();
    memcpy(data, old, oldsize);
    memcpy(data + oldsize, &second, 1);
    data[size] = '\0';
    return *this;
}

char CString::operator[] (int index) {
    return data[index];
}

CString::operator const char* () const {
    return data;
}