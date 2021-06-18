#pragma once

#include "Headers.h"
#include "String.h"

class Utils
{
public:
    static bool file_exists(const std::string& name);
    static bool file_exists(char* name);
    static wstring ExePath();
    static std::string to_string(const std::wstring& utf16Str);
    static std::string path_combine(std::string path, std::string filename);
	static std::wstring s2ws(const std::string& str);
    static String load_file(const char* file_name);
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