#include "Plugin.hpp"

#include "BranchDlg.hpp"
#include "StatusDlg.hpp"

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

void Plugin::Run(const std::string& params)
{
    GitCtl git;
    PanelInfo panelInfo;
    char cmd;

    m_far.Control(HANDLE(-1), FCTL_GETPANELINFO, &panelInfo);
    try
    {
        git.Open(panelInfo.CurDir);
    }
    catch (const char* err)
    {
        ShowError(err);
        return;
    }

    if (params == "")
    {
        std::vector <FarListItem> listItem = {
            {0, "Branch View"},
            {0, "Status View"},
        };

        FarList list = { (int)listItem.size(), listItem.data() };


        FarDialogItem dialogItems[] = {
            {DI_LISTBOX,  1,  1, 18, 10, TRUE, (DWORD_PTR)&list, DIF_LISTWRAPMODE | DIF_LISTAUTOHIGHLIGHT, 1, "Git Plugin"},
        };

        if (m_far.Dialog(m_far.ModuleNumber, -1, -1, 20, 12, NULL,
            dialogItems, sizeof(dialogItems) / sizeof(*dialogItems)) == 0)
        {
            cmd = listItem[dialogItems[0].ListPos].Text[0];
        }
    }
    else
    {
        cmd = params[0];
    }

    switch (std::tolower(cmd))
    {
    case 'b':
        BranchDlgShow(git);
        break;
    case 's':
        StatusDlgShow(git);
        break;
    }
}

void Plugin::ShowError(std::string&& message)
{
    const char* text[2] = { "Fatal error!!!", message.c_str() };
    m_far.Message(m_far.ModuleNumber, FMSG_WARNING | FMSG_MB_OK, NULL, text, 2, 0);
}

bool Plugin::ShowConfirm(std::string&& title, std::string&& text)
{
    const char* items[2];

    items[0] = title.c_str();
    items[1] = text.c_str();
    
    return (m_far.Message(m_far.ModuleNumber, FMSG_WARNING | FMSG_MB_YESNO, NULL, items, 2, 0) == 0);
}

const PluginStartupInfo* Plugin::operator->()
{
    return &m_far;
}

void Plugin::Debug(const std::string& msg)
{
    OutputDebugStringA(fmt::format("[GITCMD]{}", msg).c_str());
}
