#pragma once
#include "plugin.hpp"

#include <vector>
#include <string>

class BranchView
{
public:
	BranchView(const std::string& path);
	void Run();
private:
	GitCtl m_git;
	std::string m_currentBranch;
	std::vector<std::string> m_branches;
};