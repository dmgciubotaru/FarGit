#include "Plugin.hpp"
#include "BranchView.hpp"
#include "BranchDlg.hpp"

Plugin::Plugin()
    :m_far()
{}

Plugin& Plugin::GetPlugin()
{
	static Plugin plugin;
	return plugin;
}

void Plugin::SetInfo(const PluginStartupInfo* info)
{
	m_far = *info;
}

void Plugin::GetInfo(PluginInfo* info)
{
    info->StructSize = sizeof(*info);
    info->Flags = PF_PRELOAD | PF_DISABLEPANELS;
    info->DiskMenuStringsNumber = 0;

    info->PluginMenuStringsNumber = 0;

    info->PluginConfigStringsNumber = 0;
    info->CommandPrefix = "git";
    info->Reserved = NULL;
}

void Plugin::Exit()
{

}

void Plugin::Run(const std::string& cmd)
{
    GitCtl git;
    PanelInfo panelInfo;
    m_far.Control(HANDLE(-1), FCTL_GETPANELINFO, &panelInfo);
    try
    {
        git.Open(panelInfo.CurDir);
        BranchDlgShow(git);
    }
    catch(const char * err)
    {
        ShowError(err);
    }
}

void Plugin::ShowError(std::string&& message)
{
    const char* text[2] = { "Fatal error!!!", message.c_str() };
    m_far.Message(m_far.ModuleNumber, FMSG_WARNING | FMSG_MB_OK, NULL, text, 2, 0);
}

const PluginStartupInfo* Plugin::operator->()
{
    return &m_far;
}

void Plugin::Debug(const std::string& msg)
{
    OutputDebugStringA(fmt::format("[GITCMD]{}", msg).c_str());
}
