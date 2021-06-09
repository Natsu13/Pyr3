#include "String.h"

int String::sizeOfChar(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

void String::empty() {
    if (size > 0)
        data[0] = '\0';
}

bool String::isEmpty() {
    return data[0] == '\0';
}

void String::realoc() {
    data = (char*)malloc(size+1);
    if (data == nullptr)
        throw "Can't alloc enought memory";

    data[0] = '\0';
}

bool String::compare(char* left, char* right) {
    int i = 0;
    while (left[i] != '\0' && right[i] != '\0' && left[i] == right[i]) {
        i++;
    }
    return !(left[i] != right[i]);
}

String::String() {
    size = 0;
    realoc();
}
String::String(const char* str) {
    if(str != NULL)
        size = sizeOfChar(str);

    realoc();

    if (str != NULL) {
        memcpy(data, str, size);
        data[size] = '\0';
    }
}

String& String::operator+(String& second) {
    auto oldsize = size;
    size = size + second.size - 1;
    char* old = data;
    realoc();
    memcpy(data, old, oldsize - 1);
    memcpy(data + oldsize - 1, second.data, second.size);
    return *this;
}

String& String::operator+(const char* second) {
    if (size == 0) {
        *this = String(second);
        return *this;
    }
    auto oldsize = size;
    int sizeSecond = sizeOfChar(second);
    size = size + sizeSecond;
    char* old = data;
    realoc();
    memcpy(data, old, oldsize);
    memcpy(data + oldsize, second, sizeSecond);
    data[size] = '\0';
    return *this;
}

String& String::operator+=(char second) {
    auto oldsize = size;
    size += 1;
    char* old = data;
    realoc();
    memcpy(data, old, oldsize);
    memcpy(data + oldsize, &second, 1);
    data[size] = '\0';
    return *this;
}

String& String::operator+=(const char* second) {
    *this = *this + second;
    return *this;
}

bool String::operator==(String& second) {
    return compare(data, second.data);
}
bool String::operator!=(String& second) {
    return !compare(data, second.data);
}

char String::operator[] (int index) {
    return data[index];
}

String::operator const char* () const {
    return data;
}