#ifndef SERVER_SHARD_LUA_ENGINE_H
#define SERVER_SHARD_LUA_ENGINE_H

#include "lua_param.h"

namespace bin{
    class CScriptHandle;
}

class CLuaEngine
{
public:
	CLuaEngine(void);
	virtual ~CLuaEngine(void);

public:
	bool			            Init(std::string logpath="");

	void			            Release();
	lua_State *		            GetLuaState();
    bin::CScriptHandle*         GetScriptHandle()   { return m_ScriptHandle; }
	const char*		            GetLastError();
    void                        GcStep();

	bool			            LoadLuaFile(const char* szName);
	bool			            RunMemoryLua(const char* pLuaData, int nDataLen);
	bool			            RunLuaFunction(const char* szFunName, const char* szTableName = NULL,  const char* szSubTableName = NULL,LuaParams* pInParams=NULL, LuaParam * pOutParam = NULL, int nOutNum = 0);

    void                        ExportModule( const char* pszName );
    void                        ExportClass( const char* pszName );

private:
	lua_State*		            m_pLuaState;
    bin::CScriptHandle*         m_ScriptHandle;
};

#endif



