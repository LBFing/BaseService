#include "lua_engine.h"
#include "lua_param.h"
#include <nel/misc/displayer.h>
#include <nel/misc/path.h>
#include <server_share/pbc/pbc.h>
#include "server_share/bin_luabind/ScriptHandle.h"
#include "server_share/bin_luabind/ScriptExporter.h"

extern int  luaopen_protobuf_c (lua_State* tolua_S);
extern int  luaopen_cjson(lua_State *l);

//void Lua_Trace(lua_State *L, lua_Debug *debug_msg)
//{
//	char tmp[6]={0};
//	char * what = tmp;
//	strcpy(what,"nl\0\0");	
//	switch(debug_msg->event)
//	{
//	case LUA_HOOKCALL:
//		what = strcat(what,"uS");
//		nlinfo("LUA_HOOKCALL  ");
//		break;
//	case LUA_HOOKRET:
//		what = strcat(what,"u");
//		nlinfo("LUA_HOOKRETURN  ");
//		break;
//	case LUA_HOOKTAILRET:
//		what = strcat(what,"uS");
//		nlinfo("LUA_HOOKTAILRETURN  ");
//		break;
//	case LUA_HOOKLINE:
//		what = strcat(what,"uS");
//		nlinfo("LUA_HOOKLINE  ");
//		break;
//	case LUA_HOOKCOUNT:
//		break;
//	default:
//		break;
//	}
//	//printf("%s",what);
//	if(debug_msg->currentline >0 )	
//		nlinfo("curr line = ",debug_msg->currentline);
//
//	if(lua_getinfo(L, what, debug_msg))
//	{
//		//printf("��ʼ��%d�У�������%d�У�ʹ���ⲿ����%d��", debug_msg->linedefined,debug_msg->lastlinedefined,debug_msg->nups);
//		if(debug_msg->short_src != NULL) printf(debug_msg->short_src);
//		printf("   ");
//		if(debug_msg->what != NULL) printf(debug_msg->what);
//		printf("   ");
//		if(debug_msg->source != NULL) printf(debug_msg->source);
//		printf("   ");
//		if(debug_msg->name != NULL) printf(debug_msg->name);
//		if(debug_msg->namewhat != NULL)printf(debug_msg->namewhat);
//		printf("\n");
//	}
//	printf("\n");
//}
//
//static int Lua_Note(lua_State *L)
//{
//
//	int n = lua_gettop(L);  //�������Ĳ�������
//	int i;
//	lua_getglobal(L, "tostring"); //���ű�����tostring���ű��⺯����ѹջ
//	for (i=1; i<=n; i++)
//	{
//		const char *s;
//		lua_pushvalue(L, -1);  //��ջ���ı�������Ϊtostring �����ٴ�ѹջ
//		lua_pushvalue(L, i);   //��Ҫ��ӡ��ֵ��Ҳ���ǽű��е���ʱ����Ĳ���ѹջ
//		lua_call(L, 1, 1);     //���ýű�����tostring
//		s = lua_tostring(L, -1);  //������ֵ��ջ�ж���
//
//		if (s == NULL)
//		{
//			return luaL_error(L, "`tostring' must return a string to `print'");			
//		}
//
//		if (i>1)
//			nldebug("\t");
//
//		nldebug("%s",s);
//		lua_pop(L, 1);  //��������ֵ
//	}
//
//	nldebug("\n");
//	return 0;
//}

void PrintLuaCallstack(lua_State *L, int stack_level = 0)
{
    lua_Debug debug_msg;
    NLMISC::createDebug();

    while (lua_getstack(L, stack_level, &debug_msg) == 1)
    {
        if (lua_getinfo(L, "Slnu", &debug_msg))
        {
            NLMISC::INelContext::getInstance().getWarningLog()->setPosition(debug_msg.currentline, debug_msg.short_src, debug_msg.name);
            NLMISC::INelContext::getInstance().getWarningLog()->displayNL("");
        }
        ++stack_level;
    }
}

CLuaEngine::CLuaEngine(void)
{
	m_pLuaState = NULL;

    m_ScriptHandle = new bin::CScriptHandle();
	//CreateLuaState();
}

CLuaEngine::~CLuaEngine(void)
{
}

bool CLuaEngine::Init( std::string logpath )
{
    if(m_pLuaState!=NULL)
        return true;

    m_pLuaState = luaL_newstate();	                /// ����һ��Lua״̬����ʵ��	

    if(m_pLuaState==NULL)
    {
        nlerror( "Lua State Create Fail." );
        return false;
    }


    luaL_openlibs(m_pLuaState);                     /// ����Lua�Ļ�����

    //if( !lua_checkstack(m_pLuaState, LUA_MINSTACK) )     ///  ����Lua�Ķ�ջ��С
    //{
    //	nlerror( "set LuaState size fail(stacksize=1024)." );
    //	return false;
    //}


    //luaopen_protobuf_c(m_pLuaState);
    luaL_requiref(m_pLuaState, "protobuf.c", luaopen_protobuf_c, 0);
    luaL_requiref(m_pLuaState, "cjson", luaopen_cjson, 0);

    m_ScriptHandle->Init(m_pLuaState);

    

    //tolua_auto_lua_callback_open(m_pLuaState);

    //if( !lua_checkstack(m_pLuaState, stacksize) )     //����Lua�Ķ�ջ��С����ֹ��Ϊ��ջ��С������
    //{
    //	nldebug( "set LuaState size fail(stacksize=%d, top=%d, base=%d).", stacksize, m_pLuaState->top, m_pLuaState->base );
    //	return false;
    //}

    //#ifdef _DEBUG
    //	lua_sethook( m_pLuaState, Lua_Trace, LUA_MASKLINE , 0);
    //#endif


    //lua_register( m_pLuaState,"note",Lua_Note);

    return true;
}

void CLuaEngine::Release()
{
	if(m_pLuaState != NULL)
	{
        m_ScriptHandle->Release();
		lua_close(m_pLuaState);
	}
}

lua_State *	CLuaEngine::GetLuaState()
{
	return m_pLuaState;
}

const char* CLuaEngine::GetLastError()
{
	return lua_tostring(m_pLuaState, -1);
}

bool CLuaEngine::LoadLuaFile(const char* fileName)
{
	return m_ScriptHandle->Exec(fileName);
}


bool CLuaEngine::RunMemoryLua(const char* pLuaData, int nDataLen)
{
	if(pLuaData == NULL || nDataLen <= 0)
	{
		return false;
	}	
	int top = lua_gettop(m_pLuaState);
	if( !luaL_loadbuffer(m_pLuaState, pLuaData, nDataLen, pLuaData) )
	{
		if( !lua_pcall(m_pLuaState, 0, 0, 0) )
		{
			lua_settop(m_pLuaState, top);
			return true;
		}
	}

	nlwarning("exec memory lua error, cause %s", GetLastError());
    PrintLuaCallstack(m_pLuaState, 1);
	lua_settop(m_pLuaState, top);	
	return false;
}

bool CLuaEngine::RunLuaFunction(const char* szFunName, const char* szTableName, const char* szSubTableName, 
					LuaParams* pInParams, LuaParam * pOutParam, int nOutNum)
{
	int top = lua_gettop(m_pLuaState);
	if(szTableName==NULL)
	{
		lua_getglobal(m_pLuaState, szFunName);
	}
	else if(szSubTableName==NULL)
	{
		lua_getglobal(m_pLuaState, szTableName);
		if(lua_istable(m_pLuaState, -1))
		{
			lua_getfield(m_pLuaState,-1,szFunName);
		}
	}
	else
	{
		lua_getglobal(m_pLuaState, szTableName);
		if(lua_istable(m_pLuaState, -1))
		{
			lua_getfield(m_pLuaState, -1,szSubTableName);
			if(lua_istable(m_pLuaState, -1))
			{
				lua_getfield(m_pLuaState,-1,szFunName);
			}
		}
	}
	
	if(!lua_isfunction(m_pLuaState, -1))
	{
        nlwarning("call function(%s) fail, cause %s", szFunName, GetLastError());
        PrintLuaCallstack(m_pLuaState, 1);
		lua_settop(m_pLuaState, top);
		return false;
	}

    int inParamCnt = pInParams==NULL?0:pInParams->Count();

	for(int i = 0; i < inParamCnt; i++)
	{
		switch(pInParams->at(i).Type())
		{
			case LUA_TNUMBER:				
				//lua_pushnumber(m_pLuaState, pInParams->at(i).GetInt());
                lua_pushinteger(m_pLuaState, pInParams->at(i).GetInt());
				break;
			case LUA_TSTRING:
				lua_pushlstring(m_pLuaState, (char*)pInParams->at(i).Data(), pInParams->at(i).GetStringLen());
				break;
			case LUA_TLIGHTUSERDATA:
				lua_pushlightuserdata(m_pLuaState, pInParams->at(i).Data());
				break;
			default:
				nlwarning("call function(%s) fail, in param type error", szFunName);
                PrintLuaCallstack(m_pLuaState, 1);
				lua_settop(m_pLuaState, top);
				return false;
		}
	}

	if( !lua_pcall(m_pLuaState, inParamCnt, nOutNum, 0) )
	{
		for(int n = 0; n < nOutNum; n++)
		{
			int nType = lua_type(m_pLuaState, -1);
			switch(nType)
			{
				case LUA_TNUMBER:
					pOutParam[n].SetInt( lua_tointeger(m_pLuaState, -1) );
					lua_pop(m_pLuaState, 1);
					break;
				case LUA_TBOOLEAN:
					pOutParam[n].SetInt( lua_toboolean(m_pLuaState, -1) );
					lua_pop(m_pLuaState, 1);
					break;
				case LUA_TSTRING:

					//pOutParam[n].SetString((char*)lua_tostring(m_pLuaState, -1));
					lua_pop(m_pLuaState, 1);
					break;
				case LUA_TLIGHTUSERDATA:
					pOutParam[n].SetDataPointer((void*)lua_topointer(m_pLuaState, -1));
					lua_pop(m_pLuaState, 1);
					break;
				default:
					nlwarning("call function(%s) fail, out param type error = %s ", szFunName , lua_typename( m_pLuaState , -1 ) );
                    PrintLuaCallstack(m_pLuaState, 1);
					lua_settop(m_pLuaState, top);
					return false;					
			}
		}

		lua_settop(m_pLuaState, top);   /// �ָ�ջ��Ϊδ����ʱ��״̬��
		return true;
	}	
	
	nlwarning("call function(%s) fail, cause %s", szFunName, GetLastError());
    PrintLuaCallstack(m_pLuaState, 1);
	lua_settop(m_pLuaState, top);
	return false;
}

void CLuaEngine::ExportModule( const char* pszName )
{
    bin::ScriptExporterManager().ExportModule( pszName, *m_ScriptHandle );
}

void CLuaEngine::ExportClass( const char* pszName )
{
    bin::ScriptExporterManager().ExportClass( pszName, *m_ScriptHandle );
}

void CLuaEngine::GcStep()
{
    H_AUTO(CLuaEngineGcStep);
    lua_gc(m_pLuaState, LUA_GCSTEP, 0);
}






