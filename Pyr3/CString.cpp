#include "CString.h"

int CString::sizeOfChar(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i + 1;
}

void CString::empty() {
    if (size > 0)
        data[0] = '\0';
}

void CString::realoc() {
    data = (char*)malloc(size);
    data[0] = '\0';
}

CString::CString() {
    size = 1;
    realoc();
}
CString::CString(const char* str) {
    size = sizeOfChar(str);
    realoc();
    memcpy(data, str, size);
    data[size] = '\0';
}

CString& CString::operator+(CString& second) {
    int oldsize = size;
    size = size + second.size - 1;
    char* old = data;
    realoc();
    memcpy(data, old, oldsize - 1);
    memcpy(data + oldsize - 1, second.data, second.size);
    return *this;
}

CString& CString::operator+(const char* second) {
    int oldsize = size;
    int sizeSecond = sizeOfChar(second);
    size = size + sizeSecond - 1;
    char* old = data;
    realoc();
    memcpy(data, old, oldsize - 1);
    memcpy(data + oldsize - 1, second, sizeSecond);
    return *this;
}

char CString::operator[] (int index) {
    return data[index];
}

CString::operator const char* () const {
    return data;
}