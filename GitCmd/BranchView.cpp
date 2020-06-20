#include "BranchView.hpp"


BranchView::BranchView(const std::string& path)
{
	m_git.Open(path);
	m_currentBranch = m_git.GetCurrentBranch();
	m_branches = m_git.GetBranches();
}

void BranchView::Run()
{
	Plugin& plugin = Plugin::GetPlugin();
	
	std::vector <FarListItem>m_farItems;
	
	for (auto b : m_branches)
	{
		m_farItems.push_back({});
		m_farItems.back().Flags = b == m_currentBranch ? LIF_SELECTED : 0;
		strcpy(m_farItems.back().Text, b.c_str());
	}
	
	FarList list;
	list.ItemsNumber = (int)m_farItems.size();
	list.Items = m_farItems.data();

	FarDialogItem items[1] = {
		{DI_LISTBOX, 0, 0, 80, 17,TRUE, (DWORD_PTR)&list, DIF_LISTWRAPMODE },
	};
	plugin.Debug("OK1");

	auto ret = plugin->Dialog(plugin->ModuleNumber, -1, -1, 80, 20, NULL, items, sizeof(items) / sizeof(*items));
	plugin.Debug("OK2");
	if (ret != -1)
	{
		std::string selected = m_farItems[items[0].ListPos].Text;
		if (selected != m_currentBranch)
		{
			try
			{
				m_git.Checkout(selected);
			}
			catch (int err)
			{
				OutputDebugStringA(fmt::format("Git error {}", err).c_str());
			}
		}
	}

}
