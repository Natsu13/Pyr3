#pragma once

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <cstring>
#include <cwchar>
#include <cassert>
#include <codecvt>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

#define ASSERT(Expr, Msg) __M_Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
void __M_Assert(const char* expr_str, bool expr, const char* file, int line, const char* msg);

#define COMPARE(a,b) a[0]==b[0] && strcmp(a, b)==0
#define COMPARE_CH(a, b) a == b

#define For(elements) \
	auto it = elements.size() == 0? NULL: elements[0]; \
	for(int it_index = 0; it_index < elements.size(); it_index++,  it = elements[it_index < elements.size()? it_index: 0])

#define ForType(elements, type) for(type* it = elements.size() == 0? NULL: elements[0], int it_index = 0; it_index < elements.size(); it_index++,  it = elements[it_index < elements.size()? it_index: 0])
#define ForPush(elements, destination) for(int it_index = 0; it_index < elements.size(); it_index++) { destination.push_back(elements[it_index]); }