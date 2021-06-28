#pragma once

#include "Headers.h"
#include "String.h"
#include <functional>

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

namespace DLLCALL
{
	using namespace std;

	template<int ...> struct seq {};
	template<int N, int ...S> struct gens : gens < N - 1, N - 1, S... > {};
	template<int ...S> struct gens <0, S... >
	{
		typedef seq<S...> type;
	};

	template<typename R, int ...S, typename...Args>
	R ActualCall(seq<S...>, std::tuple<Args...> tpl, std::function<R(Args...)> func)
	{
		// It calls the function while expanding the std::tuple to it's arguments via std::get<S>
		return func(std::get<S>(tpl) ...);
	}

#pragma warning(disable:4290)

	// Functions to be called from your code
	template <typename R, typename...Args> R DLLCall(HMODULE hModule,
		const char* procname, std::tuple<Args...> tpl) throw(HRESULT)
	{
		if (!hModule)
			throw "No dll loaded";

		FARPROC fp = GetProcAddress(hModule, procname);
		if (!fp)
			throw E_POINTER;
		typedef R(__stdcall* function_pointer)(Args...);
		function_pointer P = (function_pointer)fp;

		std::function<R(Args...)> f = P;
		R return_value = ActualCall(typename gens<sizeof...(Args)>::type(), tpl, f);
		//FreeLibrary(hModule);
		return return_value;
	}
}