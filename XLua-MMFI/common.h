#ifndef COMMON_H_
#define COMMON_H_

#include "xlua.h"
#include "enum.h"
//#include "container.h"

#include <list>

typedef std::list<void*> GenList;

#define EF_ISHWA 112

#define LPARAM_TABLE 1
#define LPARAM_KEY	2
#define PARAM1 3
#define PARAM2 4
#define PARAM3 5

#define ANY_CLASS -1
#define SAME_CLASS -2
#define DIFF_CLASS -3

#define ANY_LAYER -1
#define SAME_LAYER -2
#define DIFF_LAYER -3

// Object Types

const int TYPE_OBJECT = 1;
const int TYPE_CLASS = 2;
const int TYPE_OBJECT_VALS = 3;
const int TYPE_STATIC = 4;
const int TYPE_STATIC_VALS = 5;

const int TYPE_SURFACE = 6;

// Object Upvalue Constants

const int UV_TYPE = 1;

const int UV_OBJECT_LPHO = 2;
const int UV_OBJECT_FIXED = 3;
const int UV_OBJECT_LPRH = 4;
const int UV_OBJECT_CLASS = 5;

const int UV_CLASS_LPOIL = 2;
const int UV_CLASS_LPRH = 3;

const int UV_STATIC_RH = 2;

const int UV_SURFACE_USF = 2;

// Upvalues for New* functions

const int UV_POOL_OBJECT = 1;
const int UV_POOL_CLASS = 2;

const char* const KEY_POOL_OBJECT = "xlua_mmfi_pool_object";
const char* const KEY_POOL_CLASS = "xlua_mmfi_pool_class";
const char* const KEY_TYPE_REGISTRY = "xlua_mmfi_type_registry";

// Utility Functions

int ReadOnlyViolation (lua_State* L, const char* key);
int FunctionViolation (lua_State* L, const char* key);
int MinParamCount (lua_State* L, const char* func, int count);

int ObjectCheck	(LPHO ho, LPRH rh, DWORD fixed);
int	ObjectCheck (lua_State* L);
int	ObjectCheck (lua_State* L, int index);

int	MemoClosure (lua_State* L, const char* key, lua_CFunction lfunc, int closureSize);

// Struct Resolution

#ifdef UNICODE
#error Unicode support must be handled at run-time
#endif

#pragma pack(push, 2) // grr!

// 2.5+ unlimited values and strings
typedef struct tagRV25P {
	CValue* rvpValues;
	int		rvNumberOfValues;
	int		rvFree1[VALUES_NUMBEROF_ALTERABLE - 2];		// for compatiblity with old extensions
	int		rvValueFlags;
	BYTE	rvFree2[VALUES_NUMBEROF_ALTERABLE];			// for compatiblity with old extensions
	LPTSTR* rvpStrings;
	int		rvNumberOfStrings;
	LPTSTR	rvFree3[STRINGS_NUMBEROF_ALTERABLE - 2];		// for compatiblity with old extensions
} rVal25P;

// 2.5 unlimited values
typedef struct tagRV25 {
	CValue* rvpValues;	
	long	rvNumberOfValues;
	long	rvFree1[VALUES_NUMBEROF_ALTERABLE - 2];
	long	rvValueFlags;
	BYTE	rvFree2[VALUES_NUMBEROF_ALTERABLE];
	LPTSTR	rvStrings[STRINGS_NUMBEROF_ALTERABLE];
} rVal25;

// 2.0 >= build 243, limited values, can be floating point values
typedef struct tagRV20b {
	CValue* rvpValues;
	long	rvFree1[VALUES_NUMBEROF_ALTERABLE - 1];
	long	rvValueFlags;
	BYTE	rvFree2[VALUES_NUMBEROF_ALTERABLE];
	LPSTR	rvStrings[STRINGS_NUMBEROF_ALTERABLE];
} rVal20b;

#pragma pop

headerObject*			GetHO (LPRO obj);
tagRCOM*				GetRCOM (LPHO ho);
tagRM*					GetRM (LPHO ho);
tagRA*					GetRA (LPHO ho);
tagRSPR*				GetRSPR (LPHO ho);
tagRV*					GetRV (LPHO ho);

inline tagRCOM*			GetRCOM (LPRO obj)		{ return GetRCOM(GetHO(obj)); }
inline tagRM*			GetRM (LPRO obj)		{ return GetRM(GetHO(obj)); }
inline tagRA*			GetRA (LPRO obj)		{ return GetRA(GetHO(obj)); }
inline tagRSPR*			GetRSPR (LPRO obj)		{ return GetRSPR(GetHO(obj)); }
inline tagRV*			GetRV (LPRO obj)		{ return GetRV(GetHO(obj)); }

inline headerObject*	GetHO (void* obj)		{ return GetHO((LPRO)obj); }
inline tagRCOM*			GetRCOM (void* obj)		{ return GetRCOM((LPRO)obj); }
inline tagRM*			GetRM (void* obj)		{ return GetRM((LPRO)obj); }
inline tagRA*			GetRA (void* obj)		{ return GetRA((LPRO)obj); }
inline tagRSPR*			GetRSPR (void* obj)		{ return GetRSPR((LPRO)obj); }
inline tagRV*			GetRV (void* obj)		{ return GetRV((LPRO)obj); }

// For supporting lists of functions

struct FunctionPair {
	char*			key;
	lua_CFunction	func;
};

// FindFunction will find a key in a FunctionPair list that has
// been presorted alphabetically

lua_CFunction FindFunction (const char* key, const FunctionPair* fp, int fpSize);

// Index and NewIndex are mostly the same between each common struct

template <int N1, int N2>
int StandardIndex (lua_State* L, const FunctionPair (&fpRead)[N1], const FunctionPair (&fpWrite)[N2]);

template <int N1, int N2>
int StandardNewIndex (lua_State* L, const FunctionPair (&fpRead)[N1], const FunctionPair (&fpWrite)[N2]);

#include "inline.h"

#endif