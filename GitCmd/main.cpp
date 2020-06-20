#include <windows.h>
#include "plugin.hpp"


extern "C"
{

    int WINAPI GetMinFarVersion()
    {
        return MAKEFARVERSION(3, 0, 0);
    }

    void WINAPI ExitFAR()
    {
        Plugin::GetPlugin().Exit();
    }

    void WINAPI GetPluginInfo(struct PluginInfo* Info)
    {
        Plugin::GetPlugin().GetInfo(Info);
    }

    void WINAPI SetStartupInfo(const struct PluginStartupInfo* Info)
    {
        Plugin::GetPlugin().SetInfo(Info);
    }

    HANDLE WINAPI OpenPlugin(int OpenFrom, INT_PTR Item)
    {
        Plugin::GetPlugin().Debug("enter");
        switch (OpenFrom)
        {
        case(OPEN_COMMANDLINE):
        {
            Plugin::GetPlugin().Run((char*)Item);
            return 0;
        }
        }
        return(INVALID_HANDLE_VALUE);
    }

}