#include "StatusDlg.hpp"
#include "stdlib.h"

static Plugin& plugin = Plugin::GetPlugin();

static std::vector <FarListItem> listItem;
static FarList list;

static GitCtl* pGitCtl;

#define NAME_OFFSET 6

void StatusDiff(const std::string& path, bool cached)
{
	std::string s = fmt::format("cd \"{}..\" && git diff {} \"{}\" > NUL",pGitCtl->GetRepoRoot(), 
		cached ? "--cached" : "", path);
	plugin.Debug(s.c_str());
	system(s.c_str());
}

void StatusListFilter(HANDLE hDlg)
{
	char filter[1024];
	FarDialogItemData filterData = { 1024, (char*)&filter };

	// Get filter string
	filterData.PtrLength = (int)plugin->SendDlgMessage(hDlg, DM_GETTEXT, 1, NULL);
	plugin->SendDlgMessage(hDlg, DM_GETTEXT, 1, (LONG_PTR)&filterData);

	for (auto& item : listItem)
	{
		std::string s1, s2;
		s1 = &item.Text[NAME_OFFSET];
		s2 = filter;
		std::transform(s1.begin(), s1.end(), s1.begin(), [](unsigned char c) { return std::toupper(c); });
		std::transform(s2.begin(), s2.end(), s2.begin(), [](unsigned char c) { return std::toupper(c); });

		if (std::string(s1).find(s2) == std::string::npos)
		{
			item.Flags |= LIF_HIDDEN;
		}
		else
		{
			item.Flags &= ~LIF_HIDDEN;
		}
	}

	// Update list content
	plugin->SendDlgMessage(hDlg, DM_LISTDELETE, 0, NULL);
	plugin->SendDlgMessage(hDlg, DM_LISTADD, 0, (LONG_PTR)&list);
}

void StatusListUpdate()
{
	listItem.clear();

	for (auto b : pGitCtl->GetStatus())
	{
		listItem.push_back({});
		strcpy(listItem.back().Text, fmt::format("{}|{}|{} {}", 
			b.status[GIT_FILE_STATUS_CONFLICT], 
			b.status[GIT_FILE_STATUS_INDEX], 
			b.status[GIT_FILE_STATUS_WORKSPACE], 
			b.path).c_str());
	}

	list.ItemsNumber = (int)listItem.size();
	list.Items = listItem.data();
}


LONG_PTR WINAPI StatusDlgCB(HANDLE hDlg, int msg, int par1, LONG_PTR par2)
{	
	switch (msg)
	{
	case DN_KEY:
	{
		if ((par2 >= 0x20 && par2 <= 0x7F) || par2 == '\b')
		{
			// Send user key
			plugin->SendDlgMessage(hDlg, DM_SETFOCUS, 1, 0);
			plugin->SendDlgMessage(hDlg, DM_KEY, 1, (LONG_PTR)&par2);
			plugin->SendDlgMessage(hDlg, DM_SETFOCUS, 0, 0);

			// Set hidden items
			StatusListFilter(hDlg);

			return TRUE;
		}

		switch (par2)
		{
		case KEY_F3:
		case KEY_F3 | KEY_SHIFT:
		{
			FarListPos listPos;
			plugin->SendDlgMessage(hDlg, DM_LISTGETCURPOS, 0, (LONG_PTR)&listPos);
			if (listPos.SelectPos >= 0)
			{
				StatusDiff(&listItem[listPos.SelectPos].Text[NAME_OFFSET], par2 & KEY_SHIFT);
			}

			return TRUE;
		}
		case KEY_DEL:
		{
			FarListPos listPos;
			plugin->SendDlgMessage(hDlg, DM_LISTGETCURPOS, 0, (LONG_PTR)&listPos);
			if (listPos.SelectPos >= 0)
			{
				std::string path = &listItem[listPos.SelectPos].Text[NAME_OFFSET];

				if (plugin.ShowConfirm("File checkout", fmt::format("Revert \"{}\" to HEAD?", path)))
				{
					pGitCtl->CheckoutPath(path);
					StatusListUpdate();
					StatusListFilter(hDlg);
				}
			}

			return TRUE;
		}

		case KEY_INS:
		{
			FarListPos listPos;
			plugin->SendDlgMessage(hDlg, DM_LISTGETCURPOS, 0, (LONG_PTR)&listPos);
			if (listPos.SelectPos >= 0)
			{
				std::string path = &listItem[listPos.SelectPos].Text[NAME_OFFSET];

				try
				{
					pGitCtl->StageFile(path);
				}
				catch (const char* ex)
				{
					plugin.ShowError(ex);
					return TRUE;
				}

				StatusListUpdate();
				StatusListFilter(hDlg);
				plugin->SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, (LONG_PTR)&listPos);

			}
		}

		default:
			return plugin->DefDlgProc(hDlg, msg, par1, par2);
		}
	}
	default:
		return plugin->DefDlgProc(hDlg, msg, par1, par2);
	}

}


void StatusDlgShow(GitCtl& gitCtl)
{
	pGitCtl = &gitCtl;

	StatusListUpdate();

	FarDialogItem dialogItems[] = {
		{DI_LISTBOX  ,  1,  1, 98, 24, TRUE, (DWORD_PTR)&list, DIF_LISTWRAPMODE, 0, "Status List"},
		{DI_FIXEDIT  , 10, 27, 30, 27, FALSE, NULL},
		{DI_TEXT     ,  2, 26, 97, 26, FALSE, NULL},

		{DI_DOUBLEBOX,  1, 25, 98, 28, FALSE, NULL},
		{DI_TEXT     ,  2, 27, 8 , 27, FALSE, NULL, 0, 0, "Filter:"},
	};

	strcpy(dialogItems[2].Data, ("Current Branch: " + gitCtl.GetCurrentBranch()).c_str());

	auto ret = plugin->DialogEx(plugin->ModuleNumber, -1, -1, 100, 30, NULL, dialogItems,
		sizeof(dialogItems) / sizeof(*dialogItems), NULL, 0, StatusDlgCB, NULL);
	if (ret == 0)
	{

		std::string selected = listItem[dialogItems[0].ListPos].Text;
		gitCtl.Checkout(selected);

		plugin.Debug(fmt::format("Selected {}", selected));
	}
}
