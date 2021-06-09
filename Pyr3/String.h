#pragma once
#include <Windows.h>

class String
{
private:
    int sizeOfChar(const char* str);
    void empty();
    void realoc();
    bool compare(char* left, char* right);
public:
    char* data;
    long long size = 0;

    String();
    String(const char* str);
    String& operator+(String& second);
    String& operator+(const char* second);
    String& operator+=(char second);
    String& operator+=(const char* second);
    bool operator==(String& second);
    bool operator!=(String& second);
    char operator[] (int index);
    operator const char* () const;
    bool isEmpty();
};

