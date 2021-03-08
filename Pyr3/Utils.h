#pragma once

#include "Headers.h"
#include "CString.h"

class Utils
{
public:
    static bool file_exists(const std::string& name);
    static bool file_exists(char* name);
    static wstring ExePath();
    static std::string to_string(const std::wstring& utf16Str);
    static std::string path_combine(std::string path, std::string filename);
	static std::wstring s2ws(const std::string& str);
    static CString load_file(const char* file_name);
    static bool strContChar(const char* text, char find);
    static bool strContWChar(const wchar_t* text, char find);
};

typedef union
{
	float f;
	struct
	{
		unsigned int mantissa : 23;
		unsigned int exponent : 8;
		unsigned int sign : 1;
	} field;
} myfloat;

char* intToBinary(int binary, int size);
int charbinToInt(char* bin, int size);
int floatToInteger(float f);
float integerToFloat(int64_t ieee754_bits);

/*
struct cstring {
public:
	cstring(char* text) {
		data = text;
		_size();
	}
	cstring(const char* text):cstring((char*)text) { }

	~cstring() {
		if(count > 0)
			free(data);
	}

	char* data = nullptr;
	int count = 0;

	cstring* operator+ (cstring* right) {
		if (right->count == 0)
			return this;
		if (count == 0)
			return right;

		void* str = malloc((count + right->count) * sizeof(char));
		if (str == nullptr)
			throw "Can't alloc enought memory";

		memcpy(str, data, count * sizeof(char));
		memcpy(&data[count], right->data, right->count * sizeof(char));

		return new cstring((char*)str);
	}

	char operator[] (int index) {
		if (count > index) return '\0';
		return data[index];
	}

	cstring* operator= (const char* text) {		
		return new cstring((char*)text);
	}

	cstring* operator()(const char* text) {
		return new cstring((char*)text);
	}

private:
	void _size() {
		int size = 0;
		while (data[size] != '\0') {
			size++;
		}
		count = size;
	}
};
*/