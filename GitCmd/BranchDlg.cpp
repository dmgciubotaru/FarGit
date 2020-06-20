#include "BranchDlg.hpp"

#include <map>
#include <memory>

static Plugin& plugin = Plugin::GetPlugin();

static std::vector <FarListItem> listItem;
static FarList list;

static GitCtl* pGitCtl;

void BranchListUpdate()
{
	listItem.clear();

	auto currentBranch = pGitCtl->GetCurrentBranch();

	for (auto b : pGitCtl->GetBranches())
	{
		if (b != currentBranch)
		{
			listItem.push_back({});
			strcpy(listItem.back().Text, b.c_str());
		}
	}

	list.ItemsNumber = (int)listItem.size();
	list.Items = listItem.data();
}

void BranchListFilter(HANDLE hDlg)
{
	char filter[1024];
	FarDialogItemData filterData = { 1024, (char*)&filter };

	// Get filter string
	filterData.PtrLength = (int)plugin->SendDlgMessage(hDlg, DM_GETTEXT, 1, NULL);
	plugin->SendDlgMessage(hDlg, DM_GETTEXT, 1, (LONG_PTR)&filterData);

	for (auto& item : listItem)
	{
		std::string s1, s2;
		s1 = item.Text;
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

LONG_PTR WINAPI BranchDlgCB(HANDLE hDlg, int msg, int par1, LONG_PTR par2)
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
			BranchListFilter(hDlg);
			
			return TRUE;
		}

		switch (par2)
		{
		case KEY_INS:
		{
			char name[1024];
			if (plugin->InputBox("New Branch", "Branch Name", NULL, NULL, name, 1024, NULL, NULL))
			{
				return plugin->SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
			}
			return plugin->DefDlgProc(hDlg, msg, par1, par2);
		}

		case KEY_DEL:
		{
			const char* Msg[5];
			FarListPos listPos;

			plugin->SendDlgMessage(hDlg, DM_LISTGETCURPOS, 0, (LONG_PTR)&listPos);
			if (listPos.SelectPos < 0)
			{
				return plugin->DefDlgProc(hDlg, msg, par1, par2);
			}

			Msg[0] = "Delete";
			Msg[1] = "Do you wish to delete the branch";
			Msg[2] = listItem[listPos.SelectPos].Text;
			Msg[3] = "Delete";
			Msg[4] = "Cancel";

			if (plugin->Message(plugin->ModuleNumber, 0, "DeleteFile", Msg,
				sizeof(Msg) / sizeof(Msg[0]), 2) == 0)
			{
				try 
				{
					pGitCtl->DeleteBranch(listItem[listPos.SelectPos].Text);
				}
				catch (const char* err)
				{
					plugin.ShowError(err);
					return plugin->DefDlgProc(hDlg, msg, par1, par2);
				}
			}

			BranchListUpdate();
			BranchListFilter(hDlg);

			return plugin->DefDlgProc(hDlg, msg, par1, par2);
		}


		default:
			return plugin->DefDlgProc(hDlg, msg, par1, par2);
		}
	}
	default:
		return plugin->DefDlgProc(hDlg, msg, par1, par2);
	}
	
}

void BranchDlgShow(GitCtl& gitCtl)
{
	pGitCtl = &gitCtl;

	BranchListUpdate();

	FarDialogItem dialogItems[] = {
		{DI_LISTBOX  ,  1,  1, 98, 24, TRUE, (DWORD_PTR)&list, DIF_LISTWRAPMODE, 0, "Branch List"},
		{DI_FIXEDIT  , 10, 27, 30, 27, FALSE, NULL},
		{DI_TEXT     ,  2, 26, 97, 26, FALSE, NULL},

		{DI_DOUBLEBOX,  1, 25, 98, 28, FALSE, NULL},
		{DI_TEXT     ,  2, 27, 8 , 27, FALSE, NULL, 0, 0, "Filter:"},
	};

	strcpy(dialogItems[2].Data, ("Current Branch: " + gitCtl.GetCurrentBranch()).c_str());

	auto ret = plugin->DialogEx(plugin->ModuleNumber, -1, -1, 100, 30, NULL, dialogItems, 
		sizeof(dialogItems) / sizeof(*dialogItems),	NULL,0, BranchDlgCB,NULL);
	if (ret == 0)
	{
		
		std::string selected = listItem[dialogItems[0].ListPos].Text;
		gitCtl.Checkout(selected);

		plugin.Debug(fmt::format("Selected {}", selected));
	}
}