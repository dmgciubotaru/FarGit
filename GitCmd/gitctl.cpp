#include "gitctl.hpp"

#include "..\include\fmt\format.h"

#include <stdexcept>

int gitInit = git_libgit2_init();

#define CHECK(x) {int err; if(err = (x)){throw giterr_last()->message;}}

GitCtl::GitCtl()
	:m_repo(nullptr)
{

}

void GitCtl::Open(const std::string& path)
{
	CHECK(git_repository_open(&m_repo, GetRepoRoot(path).c_str()));
}

std::vector<std::string> GitCtl::GetBranches()
{
	std::vector<std::string> names;
	git_branch_iterator* iter;
	CHECK(git_branch_iterator_new(&iter, m_repo, GIT_BRANCH_LOCAL));
	
	const char* name;
	git_reference *ref;
	git_branch_t type;
	while (!git_branch_next(&ref, &type, iter))
	{
		CHECK(git_branch_name(&name, ref));
		names.push_back(name);
	}

	git_branch_iterator_free(iter);

	return names;
}

std::string GitCtl::GetCurrentBranch()
{
	git_reference *ref;
	CHECK(git_repository_head(&ref, m_repo));
	if (git_reference_is_branch(ref))
	{
		const char* name = nullptr;
		git_branch_name(&name, ref);
		return name;
	}
	return "";
}

void GitCtl::Checkout(const std::string& branch)
{
	git_object *tree;
	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;

	CHECK(git_revparse_single(&tree, m_repo, branch.c_str()));
	CHECK(git_checkout_tree(m_repo, tree, &opts));
	
	git_reference* ref;
	GetBranchRef(ref, branch);
	CHECK(git_repository_set_head(m_repo, git_reference_name(ref)));
}

void GitCtl::DeleteBranch(const std::string& branch)
{
	if (branch == "master")
	{
		throw "Refuse to delete master branch";
	}

	git_reference* branchRef;
	git_reference* masterRef;

	GetBranchRef(branchRef, branch);
	GetBranchRef(masterRef, "master");

	git_oid mergeBase;
	git_oid targetCommit;
	memcpy(&targetCommit, git_reference_target(branchRef), sizeof(git_oid));


	// Check if the branch was merged to master
	CHECK(git_merge_base(&mergeBase, m_repo, git_reference_target(masterRef), &targetCommit));
	if (memcmp(&mergeBase, &targetCommit, sizeof(git_oid)) != 0)
	{
		throw "Refuse to delete unmerged branch";
	}
	
	CHECK(git_branch_delete(branchRef));
}

std::vector<GitFileStatus> GitCtl::GetStatus()
{
	std::vector<GitFileStatus> statuses;
	CHECK(git_status_foreach(m_repo, GetStatusCB, &statuses));
	return statuses;
}

void GitCtl::CheckoutPath(const std::string& path)
{
	git_checkout_options opt;
	const char* pathStr = path.c_str();
	CHECK(git_checkout_options_init(&opt, GIT_CHECKOUT_OPTIONS_VERSION));
	opt.paths.strings = (char **) &pathStr;
	opt.paths.count = 1;
	opt.checkout_strategy = GIT_CHECKOUT_FORCE;
	CHECK(git_checkout_head(m_repo, &opt));
}

void GitCtl::StageFile(const std::string& path)
{
	git_index *index;
	CHECK(git_repository_index(&index, m_repo));
	CHECK(git_index_add_bypath(index, path.c_str()));
	CHECK(git_index_write(index));
}

std::string GitCtl::GetRepoRoot(const std::string& path)
{
	git_buf root;
	std::string rootStr;

	root.size = 1024;
	root.asize = 1024;
	root.ptr = (char *)malloc(root.size);

	CHECK(git_repository_discover(&root, path.c_str(), 0, NULL));
	
	rootStr = root.ptr;
	free(root.ptr);
	return rootStr;
}

void GitCtl::GetBranchRef(git_reference*& ref, const std::string& branch)
{
	CHECK(git_branch_lookup(&ref, m_repo, branch.c_str(), GIT_BRANCH_LOCAL));
}

int GitCtl::GetStatusCB(const char* path, unsigned int status_flags, void* payload)
{
	std::vector<GitFileStatus>* statuses = (std::vector<GitFileStatus>*)payload;
	if (status_flags != GIT_STATUS_IGNORED)
	{
		GitFileStatus status = {{GIT_FILE_STATUS_NONE, GIT_FILE_STATUS_NONE, GIT_FILE_STATUS_NONE}, path};
		switch (status_flags & 0x1f)
		{
		case 0:
		{
			break;
		}
		case GIT_STATUS_INDEX_NEW:
		{
			status.status[GIT_FILE_STATUS_INDEX] = 'N';
			break;
		}
		case GIT_STATUS_INDEX_MODIFIED:
		{
			status.status[GIT_FILE_STATUS_INDEX] = 'M';
			break;
		}
		case GIT_STATUS_INDEX_DELETED:
		{
			status.status[GIT_FILE_STATUS_INDEX] = 'D';
			break;
		}
		case GIT_STATUS_INDEX_RENAMED:
		{
			status.status[GIT_FILE_STATUS_INDEX] = 'R';
			break;
		}
		case GIT_STATUS_INDEX_TYPECHANGE:
		{
			status.status[GIT_FILE_STATUS_INDEX] = 'T';
			break;
		}
		default:
			throw "Multiple flags found";
		}
		
		switch (status_flags & (0x3f << 7))
		{
		case 0:
		{
			break;
		}
		case GIT_STATUS_WT_NEW:
		{
			status.status[GIT_FILE_STATUS_WORKSPACE] = 'N';
			break;
		}
		case GIT_STATUS_WT_MODIFIED:
		{
			status.status[GIT_FILE_STATUS_WORKSPACE] = 'M';
			break;
		}
		case GIT_STATUS_WT_DELETED:
		{
			status.status[GIT_FILE_STATUS_WORKSPACE] = 'D';
			break;
		}
		case GIT_STATUS_WT_RENAMED:
		{
			status.status[GIT_FILE_STATUS_WORKSPACE] = 'R';
			break;
		}
		case GIT_STATUS_WT_TYPECHANGE:
		{
			status.status[GIT_FILE_STATUS_WORKSPACE] = 'T';
			break;
		}
		case GIT_STATUS_WT_UNREADABLE:
		{
			status.status[GIT_FILE_STATUS_WORKSPACE] = 'U';
			break;
		}
		default:
			throw "Multiple flags found";
		}

		if (status_flags & GIT_STATUS_CONFLICTED)
		{
			status.status[GIT_FILE_STATUS_CONFLICT] = 'C';
		}
		statuses->push_back(status);
	}
	return 0;
}
