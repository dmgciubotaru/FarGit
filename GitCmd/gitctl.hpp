#pragma once

#include "..\include\libgit2\git2.h"

#include <string>
#include <vector>

struct GitFileStatus
{
	std::string path;
	git_status_t status;
};

class GitCtl
{
public:
	GitCtl();

	void Open(const std::string& path);
	const std::string& GetLastError();

	// Branches
	std::vector<std::string> GetBranches();
	std::string GetCurrentBranch();
	void Checkout(const std::string& branch);
	void DeleteBranch(const std::string& branch);
	


	// Status
	std::vector<GitFileStatus> GetStatus();


	static std::string GetRepoRoot(const std::string& path);

private:
	git_repository* m_repo;
	std::string m_lastError;

	void GetBranchRef(git_reference*& ref,const std::string& branch);
	static int GetStatusCB(const char* path, unsigned int status_flags, void* payload);
};