﻿#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <nel/misc/window_displayer.h>
#include <server_share/server_def.h>
#include <server_share/lua/lua_engine.h>
#include <server_share/lua/script_mgr.h>
#include <server_share/lua/lua_thread.h>
#include <server_share/lua_net/lua_network.h>
#include <server_share/i18n_def.h>
#include <server_share/timer.h>
#include <nel/net/naming_client.h>
#include <server_share/lua_net/lua_callback_client.h>

using namespace NLMISC;
using namespace NLNET;
using namespace std;

class CClientRobot : public NLNET::IService
{
public:

    /// Init the service, load the universal time.
    void init ()
    {
        

        TimerManager->init();
        LuaNetworkMgr.Init();
        ScriptMgr.init();
        LuaThreadMgr.Init();
        
    }


    bool update ()
    {
        NLMISC::TTicks curr_ticks = CTime::getLocalTime();
		LocalTime.SetCurrTime(curr_ticks);
        TimerManager->tickUpdate();
        ScriptMgr.update();
        LuaNetworkMgr.Update();
        LuaThreadMgr.Update();
        return true;
    }

    void release ()
    {
		TimerManager->release();
        ScriptMgr.release();
        LuaNetworkMgr.Release();
        LuaThreadMgr.Release();
    }

};

// Service instantiation
NLNET_SERVICE_MAIN (CClientRobot, "ROBOT", "client_robot", 0, EmptyCallbackArray, "", "");



