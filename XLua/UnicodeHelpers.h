#pragma once

#include <Windows.h>
#include <string>

#define TSTRING std::basic_string<TCHAR>

// Takes a string from a Fusion source (potentially UTF16) and converts it to UTF8 for Lua interaction.
// (Does nothing in Non-Unicode builds)

class TempLuaString {
public:
#ifdef UNICODE

	TempLuaString(const wchar_t* source) {
		int length = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
		str = new char[length];
		WideCharToMultiByte(CP_UTF8, 0, source, -1, str, length, NULL, NULL);
	}

	~TempLuaString() {
		delete[] str;
	}

#else

	TempLuaString(char* source) : str(source) {}

#endif

	const char* c_str() const {
		return str;
	}

	operator const char* () const {
		return str;
	}

	operator std::string() const {
		return std::string(str);
	}

private:

	char* str;
};

// Takes a string from a Lua source (interpreted as UTF8) and converts it to UTF16 so MMF can process it.
// Supports MMF's internal allocation function for string expression output.
// (Does nothing in Non-Unicode builds besides allowing MMF allocation)

class TempMMFString {
public:
	#ifdef UNICODE

	TempMMFString(const char* source) {
		int length = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
		str = new wchar_t[length];
		MultiByteToWideChar(CP_UTF8, 0, source, -1, str, length);
	}
	TempMMFString(LPRDATA rdPtr, const std::string& source) : mmfAllocated(true) {
		int length = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), source.size(), NULL, 0);
		str = (wchar_t*)callRunTimeFunction(rdPtr, RFUNCTION_GETSTRINGSPACE_EX, 0, (length + 1) * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, source.c_str(), source.size(), str, length);
		str[length] = L'\0'; // MBTOWC won't include it if we don't pass -1 as length!
	}

	~TempMMFString() {
		if (!mmfAllocated) {
			delete[] str;
		}
	}

	#else

	TempMMFString(const char* source) : str(const_cast<char*>(source)) {}
	TempMMFString(LPRDATA rdPtr, const std::string& source) : mmfAllocated(true) {
		str = (char*)callRunTimeFunction(rdPtr, RFUNCTION_GETSTRINGSPACE_EX, 0, source.size() + 1);
		memcpy(str, source.c_str(), source.size() + 1);
	}

	#endif

	const TCHAR* c_str() const {
		return str;
	}

	operator TSTRING() const {
		return TSTRING(str);
	}

private:

	TCHAR* str;
	bool mmfAllocated = false;
};
