#pragma once

#include "Headers.h"
#include "Utils.h"

bool Utils::file_exists(const std::string& name) {
    ifstream f(name.c_str());
    return f.good();
}

bool Utils::file_exists(char* name) {
    ifstream f(name);
    return f.good();
}

wstring Utils::ExePath() {
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}

std::string Utils::to_string(const std::wstring& w) {
    //std::string s((const char*)&utf16Str[0], sizeof(wchar_t) / sizeof(char) * utf16Str.size());
    std::string s(w.begin(), w.end());
    return s;
}

std::string Utils::path_combine(std::string path, std::string filename) {
    fs::path dir(path);
    fs::path file(filename);
    fs::path full_path = dir / file;
    return full_path.u8string();
}

std::wstring Utils::s2ws(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::wstring Utils::load_file(const char* file_name) {
	auto path = Utils::to_string(Utils::ExePath());
	auto path_to_file = Utils::path_combine(path, file_name);

	FILE* file;
	int err = 0;
	if ((err = fopen_s(&file, path_to_file.c_str(), "rb")) != 0) {
		ASSERT(false, "File can't open");
	}

	fseek(file, 0, SEEK_END);
	long lSize = ftell(file) + 1;
	rewind(file);

	char* buffer = (char*)malloc(sizeof(char) * lSize);// 0 terminator
	ASSERT(buffer != NULL, "Buffer is null");

	size_t result = fread(buffer, 1, lSize + 1, file);
	ASSERT(result + 1 == lSize, "Can't read file");

	buffer[lSize - 1] = 0;
	std::wstring replyStr = (Utils::s2ws((char*)buffer));

	fclose(file);
	free(buffer);

	return replyStr;
}

bool Utils::strContWChar(const wchar_t* text, char find) {
	for (size_t i = 0; i < wcslen(text); ++i) {
		if (text[i] == find) return true;
	}
	return false;
}

bool Utils::strContChar(const char* text, char find) {
	for (size_t i = 0; i < strlen(text); ++i) {
		if (text[i] == find) return true;
	}
	return false;
}

char* intToBinary(int binary, int size) {
	char* out = new char[size + 2];

	int q = 0;
	int k;
	for (size--; size >= 0; size--)
	{
		k = binary >> size;
		if (k & 1)
			out[q] = '1';
		else
			out[q] = '0';
		q++;
	}

	out[q] = '\0';

	return out;
}

int charbinToInt(char* bin, int size) {
	int ret = 0;
	int o = 1;
	for (int i = size - 1; i >= 0; i--) {
		int p = 0;
		if (bin[i] == '1') p = 1;
		ret += p * o;
		o *= 2;
	}
	return ret;
}

int floatToInteger(float f) {
	myfloat var;
	var.f = f;
	char buffer[50];
	sprintf_s(buffer, "0%s%s", intToBinary(var.field.exponent, 8), intToBinary(var.field.mantissa, 23));
	return charbinToInt(buffer, 32);
}

float integerToFloat(int64_t ieee754_bits) {
	float flt;
	*((int*)&flt) = ieee754_bits;
	return flt;
}