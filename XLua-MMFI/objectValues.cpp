#include "object.h"

// All lookup functions for values require subtables.  Create and
// memoize corresponding lookup tables so we only pay the metatable
// price once

enum {
	AV_MMF2, // Fixed number of values and strings
	AV_CF25, // Variable number of values, fixed number of strings
	AV_CF25PLUS, // Variable number of values and strings
};

inline int GetAlterablesVersion(LPHO o) {
	static DWORD mmfVersion = o->hoAdRunHeader->rh4.rh4Mv->mvGetVersion() & MMFBUILD_MASK;
	if (mmfVersion >= 292) return AV_CF25PLUS;
	if (mmfVersion >= 282) return AV_CF25;
	return AV_MMF2;
}

inline bool IsUnicode(LPHO o) {
	static bool unicode = o->hoAdRunHeader->rh4.rh4Mv->mvCallFunction(NULL, EF_ISUNICODE, 0, 0, 0);
	return unicode;
}

int Values::ValuesIndex (lua_State* L) {
	LPHO o = GetHO(lua_touserdata(L, lua_upvalueindex(UV_OBJECT_LPHO)));
	tagRV* val = GetRV(o);

	if (!val)
		return -1;

	const char* key = lua_tostring(L, 2);

	if (!(strcmp(key, "flags") == 0 || strcmp(key, "values") == 0 || strcmp(key, "strings") == 0)) {
		return -1;
	}

	int (*index_method)(lua_State*L) = 0;
	int (*newindex_method)(lua_State*L) = 0;

	if (strcmp(key, "flags") == 0) {
		index_method = Values::Flag;
		newindex_method = Values::SetFlag;
	}
	else if (strcmp(key, "values") == 0) {
		index_method = Values::Value;
		newindex_method = Values::SetValue;
	}
	else if (strcmp(key, "strings") == 0) {
		index_method = Values::String;
		newindex_method = Values::SetString;
	}

	// If we memoized the table in the metatable, return that
	lua_getmetatable(L, 1);
	lua_pushstring(L, key);
	lua_rawget(L, -2);

	if (lua_istable(L, -1)) {
		return 1;
	}

	// Return metatable to top of stack
	lua_pop(L, 1);

	// Calling table and key are allready on the stack in the order we need them
	lua_pushstring(L, key);

	// Create object table
	lua_createtable(L, 0, 0);

	// Create object metatable
	lua_createtable(L, 0, 2);

	// Add __index
	lua_pushvalue(L, TYPE_OBJECT_VALS);
	lua_pushvalue(L, lua_upvalueindex(UV_OBJECT_LPHO));
	lua_pushvalue(L, lua_upvalueindex(UV_OBJECT_FIXED));
	lua_pushvalue(L, lua_upvalueindex(UV_OBJECT_LPRH));
	//lua_pushlightuserdata(L, (void*)val);
	lua_pushcclosure(L, index_method, 4);
	lua_setfield(L, -2, "__index");

	// Add __newindex
	lua_pushvalue(L, TYPE_OBJECT_VALS);
	lua_pushvalue(L, lua_upvalueindex(UV_OBJECT_LPHO));
	lua_pushvalue(L, lua_upvalueindex(UV_OBJECT_FIXED));
	lua_pushvalue(L, lua_upvalueindex(UV_OBJECT_LPRH));
	//lua_pushlightuserdata(L, (void*)val);
	lua_pushcclosure(L, newindex_method, 4);
	lua_setfield(L, -2, "__newindex");

	// Add __metatable
	lua_pushstring(L, "Access violation");
	lua_setfield(L, -2, "__metatable");
	
	// Bind metatable
	lua_setmetatable(L, -2);

	// Set calling table
	lua_rawset(L, -3);

	// Return new table
	lua_pushstring(L, key);
	lua_rawget(L, -2);

	return 1;
}

int Values::ValuesNewIndex (lua_State* L) {
	return -1;
}

int Values::Flag (lua_State* L) {
	unsigned int key = lua_tointeger(L, 2);

	if (!ObjectCheck(L)) {
		return 0;
	}

	if (key == 0 || key > 32) {
		return 0;
	}
	key %= 32;

	LPHO ho = GetHO(lua_touserdata(L, lua_upvalueindex(UV_OBJECT_LPHO)));
	
	// Note: no need to check for MMF version here, same format for all
	tagRV* rv = GetRV(ho);
	lua_pushboolean(L, (rv->rvValueFlags & (1 << key)) != 0);

	return 1;
}

int Values::Value (lua_State* L) {
	int key = ValueKeyLookup(L);
	if (key < 0)
		return 0;

	if (!ObjectCheck(L)) {
		return 0;
	}

	LPHO ho = GetHO(lua_touserdata(L, lua_upvalueindex(UV_OBJECT_LPHO)));
	tagRV* rv = GetRV(ho);

	int altVer = GetAlterablesVersion(ho);
	int numberOfAlts = altVer == AV_MMF2 ? VALUES_NUMBEROF_ALTERABLE : ((tagRV25*)rv)->rvNumberOfValues;

	if (key >= numberOfAlts) {
		return 0;
	}

	CValue* cvalue = altVer == AV_MMF2 ? &rv->rvpValues[key] : &((tagRV25*)rv)->rvpValues[key];

	switch (cvalue->m_type) {
	case TYPE_LONG: lua_pushinteger(L, cvalue->m_long); break;
	case TYPE_FLOAT: lua_pushnumber(L, cvalue->m_double); break;
	case TYPE_STRING: {
		// Can alterables ever be strings? Perhaps using the MMFI only
		if (IsUnicode(ho)) {
			WCHAR* wideString = (WCHAR*)cvalue->m_pString;
			if (wideString == nullptr) {
				return 0;
			}
			const size_t size = wcslen(wideString) + 1;
			char* string = new char[size];
			wcstombs(string, wideString, size);
			lua_pushstring(L, string);
			delete[] string;
		}
		else {
			lua_pushstring(L, cvalue->m_pString);
		}
		break;
	}
	}

	return 1;
}

int Values::String (lua_State* L) {
	//unsigned int key = lua_tointeger(L, 2);
	int key = StringKeyLookup(L);
	if (key < 0)
		return 0;

	if (!ObjectCheck(L)) {
		return 0;
	}

	LPHO ho = GetHO(lua_touserdata(L, lua_upvalueindex(UV_OBJECT_LPHO)));
	tagRV* rv = GetRV(ho);

	auto altVer = GetAlterablesVersion(ho);
	if (altVer != AV_CF25PLUS && key >= STRINGS_NUMBEROF_ALTERABLE) {
		return 0;
	}

	LPSTR altString = nullptr;
	switch (GetAlterablesVersion(ho)) {
	case AV_MMF2: altString = rv->rvStrings[key]; break;
	case AV_CF25: altString = ((tagRV25*)rv)->rvStrings[key]; break;
	case AV_CF25PLUS: {
		auto rv25p = (tagRV25P*)rv;
		if (key >= rv25p->rvNumberOfStrings) return 0;
		altString = rv25p->rvpStrings[key];
		break;
	}
	}

	if (altString == nullptr) {
		lua_pushstring(L, "");
	}
	else if (IsUnicode(ho)) {
		WCHAR* wideString = (WCHAR*)altString;
		const size_t size = wcslen(wideString) + 1;
		char* string = new char[size];
		wcstombs(string, wideString, size);
		lua_pushstring(L, string);
		delete[] string;
	}
	else {
		lua_pushstring(L, altString);
	}

	return 1;
}

int Values::SetFlag (lua_State* L) {
	unsigned int key = lua_tointeger(L, 2);
	unsigned int bit = lua_toboolean(L, 3);

	if (!ObjectCheck(L)) {
		return 0;
	}

	if (key == 0 || key > 32) {
		return 0;
	}
	key %= 32;

	LPHO ho = GetHO(lua_touserdata(L, lua_upvalueindex(UV_OBJECT_LPHO)));
	
	// Note: no need to check for MMF version here, same format for all
	tagRV* rv = GetRV(ho);
	rv->rvValueFlags = (rv->rvValueFlags & ~(1 << key)) | (bit << key);

	return 0;
}

int Values::SetValue (lua_State* L) {
	//unsigned int key = lua_tointeger(L, 2);
	int key = ValueKeyLookup(L);
	if (key < 0)
		return 0;

	if (!ObjectCheck(L)) {
		return 0;
	}

	LPHO ho = GetHO(lua_touserdata(L, lua_upvalueindex(UV_OBJECT_LPHO)));
	tagRV* rv = GetRV(ho);

	auto altVer = GetAlterablesVersion(ho);
	if (altVer == AV_MMF2 && key >= VALUES_NUMBEROF_ALTERABLE) {
		return 0;
	}

	int intval = lua_tointeger(L, 3);
	double dblval = lua_tonumber(L, 3);

	CValue* cvalue = nullptr;
	switch (altVer) {
	case AV_MMF2:
		cvalue = &rv->rvpValues[key];
		break;
	case AV_CF25:
	case AV_CF25PLUS: {
		auto rv25 = ((tagRV25*)rv);
		if (rv25->rvNumberOfValues <= key) {
			rv25->rvpValues = (CValue*)mvReAlloc(ho->hoAdRunHeader->rh4.rh4Mv, rv25->rvpValues, sizeof(CValue) * (key + 1));
			auto oldNum = rv25->rvNumberOfValues;
			memset(rv25->rvpValues + oldNum, 0, sizeof(CValue) * (key + 1 - oldNum));
			rv25->rvNumberOfValues = key + 1;
		}
		cvalue = &rv25->rvpValues[key];
		break;
	}
	}

	// Try to figure out if it's an int or a double.
	if ((double)intval == dblval) {
		cvalue->m_type = TYPE_LONG;
		cvalue->m_long = intval;
	}
	else {
		cvalue->m_type = TYPE_DOUBLE;
		cvalue->m_double = dblval;
	}

	return 0;
}

int Values::SetString (lua_State* L) {
	//unsigned int key = lua_tointeger(L, 2);
	int key = StringKeyLookup(L);
	if (key < 0)
		return 0;

	if (!ObjectCheck(L)) {
		return 0;
	}

	LPHO ho = GetHO(lua_touserdata(L, lua_upvalueindex(UV_OBJECT_LPHO)));
	tagRV* rv = GetRV(ho);

	auto altVer = GetAlterablesVersion(ho);
	if (altVer != AV_CF25PLUS && key >= STRINGS_NUMBEROF_ALTERABLE) {
		return 0;
	}

	LPSTR* targetString = nullptr;
	switch (altVer) {
	case AV_MMF2: targetString = &rv->rvStrings[key]; break;
	case AV_CF25: targetString = &((tagRV25*)rv)->rvStrings[key]; break;
	case AV_CF25PLUS: {
		auto rv25p = (tagRV25P*)rv;
		// Variable number of strings, ensure we can fit ours
		_ASSERT(rv25p->rvpStrings);
		if (rv25p->rvNumberOfStrings <= key) {
			rv25p->rvpStrings = (LPSTR*)mvReAlloc(ho->hoAdRunHeader->rh4.rh4Mv, rv25p->rvpStrings, sizeof(LPSTR) * (key + 1));
			auto oldNum = rv25p->rvNumberOfValues;
			memset(rv25p->rvpStrings + oldNum, 0, sizeof(LPTSTR) * (key + 1 - oldNum));
			rv25p->rvNumberOfStrings = key + 1;
		}
		targetString = &rv25p->rvpStrings[key];
		break;
	}
	}

	unsigned slen = lua_objlen(L, 3);
	if (IsUnicode(ho)) {
		auto size = slen + 1;
		if (*targetString != nullptr) {
			auto prevValue = *targetString;
			*targetString = (LPSTR)mvReAlloc(ho->hoAdRunHeader->rh4.rh4Mv, *targetString, sizeof(WCHAR) * size);
		}
		else {
			*targetString = (LPSTR)mvCalloc(ho->hoAdRunHeader->rh4.rh4Mv, sizeof(WCHAR) * size);
		}
		mbstowcs((wchar_t*)*targetString, lua_tostring(L, 3), size);
	}
	else {
		if (*targetString != nullptr) {
			*targetString = (LPSTR)mvReAlloc(ho->hoAdRunHeader->rh4.rh4Mv, *targetString, slen + 1);
		}
		else {
			*targetString = (LPSTR)mvCalloc(ho->hoAdRunHeader->rh4.rh4Mv, slen + 1);
		}
		strncpy_s(*targetString, slen + 1, lua_tostring(L, 3), slen + 1);
	}

	return 0;
}

// -----

int Values::ValueKeyLookup (lua_State* L) {
	if (lua_isnumber(L, LPARAM_KEY))
		return lua_tointeger(L, LPARAM_KEY) - 1;

	if (!lua_isstring(L, LPARAM_KEY))
		return -1;

	const char* key = lua_tostring(L, LPARAM_KEY);

	LPHO ho = (LPHO) lua_touserdata(L, lua_upvalueindex(UV_OBJECT_LPHO));

	short* vnameCount = (short*)((char*)ho->hoCommon + ho->hoCommon->ocValueNames);
	char* vname = (char*)vnameCount + 2;

	for (short i = 0; i < *vnameCount; i++) {
		if (strcmp(key, vname) == 0)
			return i;

		vname += strlen(vname) + 1;
	}

	return -1;
}

int Values::StringKeyLookup (lua_State* L) {
	if (lua_isnumber(L, LPARAM_KEY))
		return lua_tointeger(L, LPARAM_KEY) - 1;

	if (!lua_isstring(L, LPARAM_KEY))
		return -1;

	const char* key = lua_tostring(L, LPARAM_KEY);

	LPHO ho = (LPHO) lua_touserdata(L, lua_upvalueindex(UV_OBJECT_LPHO));

	short* vnameCount = (short*)((char*)ho->hoCommon + ho->hoCommon->ocStringNames);
	char* vname = (char*)vnameCount + 2;

	for (short i = 0; i < *vnameCount; i++) {
		if (strcmp(key, vname) == 0)
			return i;

		vname += strlen(vname) + 1;
	}

	return -1;
}
