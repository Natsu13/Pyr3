#pragma once
#include <Windows.h>

class CString
{
private:
    int sizeOfChar(const char* str);
    void empty();
    void realoc();

public:
    char* data;
    int size = 0;

    CString();
    CString(const char* str);
    CString& operator+(CString& second);
    CString& operator+(const char* second);
    char operator[] (int index);
    operator const char* () const;
};

