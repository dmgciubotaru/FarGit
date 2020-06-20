#pragma once

#include "..\include\libgit2\git2.h"

#include <string>
#include <vector>


#define GIT_FILE_STATUS_INDEX 0
#define GIT_FILE_STATUS_WORKSPACE 1
#define GIT_FILE_STATUS_CONFLICT 2
#define GIT_FILE_STATUS_NONE '-'

struct GitFileStatus
{
	char status[3]; // Index|WorkingDir|Conflict
	std::string path;
};

class GitCtl
{
public:
	GitCtl();

	void Open(const std::string& path);

	// Branches
	std::vector<std::string> GetBranches();
	std::string GetCurrentBranch();
	void Checkout(const std::string& branch);	// Switch to target branch
	void DeleteBranch(const std::string& branch);
	
	// Status
	std::vector<GitFileStatus> GetStatus();
	void CheckoutPath(const std::string& path); // Revert path to HEAD
	void StageFile(const std::string& path);	// Stage file for commit

	static std::string GetRepoRoot(const std::string& path);

private:
	git_repository* m_repo;
	std::string m_lastError;

	void GetBranchRef(git_reference*& ref,const std::string& branch);
	static int GetStatusCB(const char* path, unsigned int status_flags, void* payload);
};