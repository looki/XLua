#ifndef XLUA_GLOBAL_H_
#define XLUA_GLOBAL_H_

#include <list>
#include <hash_map>

#include "lua.hpp"

// =====================================================================================
// XLua Global State Manager
// =====================================================================================

class XLuaState;

/**
 * XLuaGlobal is a globally instantiated object running the lifetime of both runtimes.
 * Its purpose is to bind XLuaState objects (representing independent Lua States) to
 * state identifiers, and make them available for lookup either by id, or by the Lua
 * state that they represent.
 */

class XLuaGlobal {
public:

	// Lookup tables
	stdext::hash_map<int, XLuaState*>			_stateTable;
	stdext::hash_map<lua_State*, XLuaState*>	_stateLookup;

	// Reverse lookup table
	stdext::hash_map<XLuaState*, int>			_idLookup;

#ifdef XLUA_LEGACY
	// List of XLuaStates with active WIN interfaces
	std::list<XLuaState*>						_winiStates;

	typedef std::list<XLuaState*>::iterator		IWiniState;
#endif

	typedef	stdext::hash_map<int, XLuaState*>::iterator	IStateMap;
	typedef stdext::hash_map<XLuaState*, int>::iterator	IIdMap;
	

public:

	XLuaGlobal ();
	~XLuaGlobal ();

	bool		CreateState (int sid);

	bool		DeleteState (int sid);
	bool		DeleteState (XLuaState* s);

	XLuaState*	GetState (int sid);
	XLuaState*	GetStateByState (lua_State* state);

	static XLuaGlobal& Get ();
};


//---------------------------------------------------------------------------------------
//
// Inline Implementation Below
//

//---------------------------------------------------------------------------------------
// XLuaGlobal

inline XLuaGlobal::XLuaGlobal () { }

inline XLuaGlobal::~XLuaGlobal () { }

inline XLuaState* XLuaGlobal::GetState (int sid) {
	return _stateTable[sid];
}

#define XLUA_REGISTRY_MAIN_THREAD static_cast<int>('MAIN')

inline XLuaState* XLuaGlobal::GetStateByState (lua_State* state) {
	XLuaState* ret = _stateLookup[state];
	if (!ret) {
		lua_State* coro_state = state;
		lua_pushinteger(coro_state, XLUA_REGISTRY_MAIN_THREAD);
		lua_gettable(coro_state, LUA_REGISTRYINDEX);
		state = static_cast<lua_State*>(lua_touserdata(coro_state, -1));
		lua_pop(coro_state, 1);
		ret = _stateLookup[state];
	}
	return ret;
}

inline XLuaGlobal& XLuaGlobal::Get () {
	static XLuaGlobal* xg = new XLuaGlobal();
	return *xg;
}

#endif
