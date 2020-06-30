#include "BranchDlg.hpp"
#include "utils.h"

#include <map>
#include <memory>

static Plugin& plugin = Plugin::GetPlugin();

static std::vector <FarListItem> listItem;
static FarList list;

static GitCtl* pGitCtl;


void BranchListUpdate(HANDLE hDlg)
{
	char filter[1024];
	FarDialogItemData filterData = { 1024, (char*)&filter };

	// Get filter string
	filterData.PtrLength = (int)plugin->SendDlgMessage(hDlg, DM_GETTEXT, 1, NULL);
	plugin->SendDlgMessage(hDlg, DM_GETTEXT, 1, (LONG_PTR)&filterData);

	listItem.clear();

	auto currentBranch = pGitCtl->GetCurrentBranch();

	for (auto b : pGitCtl->GetBranches())
	{
		FarListItem item{ 0 };
		strcpy(item.Text, b.c_str());

		if (ToUpper(b) == ToUpper(currentBranch))
		{
			item.Flags |= LIF_DISABLE;
		}

		if (ToUpper(b).find(ToUpper(filter)) == std::string::npos)
		{
			item.Flags |= LIF_HIDDEN;
		}

		listItem.push_back(item);
	}

	list.ItemsNumber = (int)listItem.size();
	list.Items = listItem.data();

	// Update list content
	plugin->SendDlgMessage(hDlg, DM_LISTDELETE, 0, NULL);
	plugin->SendDlgMessage(hDlg, DM_LISTADD, 0, (LONG_PTR)&list);
}

LONG_PTR WINAPI BranchDlgCB(HANDLE hDlg, int msg, int par1, LONG_PTR par2)
{
	switch (msg)
	{
	case DN_INITDIALOG:
	{
		BranchListUpdate(hDlg);
		return plugin->DefDlgProc(hDlg, msg, par1, par2);
	}

	case DN_KEY:
	{
		if ((par2 >= 0x20 && par2 <= 0x7F) || par2 == '\b')
		{
			// Send user key
			plugin->SendDlgMessage(hDlg, DM_SETFOCUS, 1, 0);
			plugin->SendDlgMessage(hDlg, DM_KEY, 1, (LONG_PTR)&par2);
			plugin->SendDlgMessage(hDlg, DM_SETFOCUS, 0, 0);

			// Set hidden items
			BranchListUpdate(hDlg);
			
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
			FarListPos listPos;

			plugin->SendDlgMessage(hDlg, DM_LISTGETCURPOS, 0, (LONG_PTR)&listPos);
			if (listPos.SelectPos < 0)
			{
				return plugin->DefDlgProc(hDlg, msg, par1, par2);
			}

			if(plugin.ShowConfirm("Confirm branch delete", 
				fmt::format("Delete branch \"{}\"?", listItem[listPos.SelectPos].Text)))
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

			BranchListUpdate(hDlg);

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
		try
		{
			gitCtl.Checkout(selected);
		}
		catch (const char* err)
		{
			plugin.ShowError(err);
		}

		plugin.Debug(fmt::format("Selected {}", selected));
	}
}