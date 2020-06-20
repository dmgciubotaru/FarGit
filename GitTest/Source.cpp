#include "..\GitCmd\gitctl.hpp"

#include <exception>
#include <stdexcept>
#include <stdio.h>

//$(SolutionDir)\libgit2

void test();
int main()
{
	try 
	{
		test();
	}
	catch (const std::runtime_error& ex)
	{
		printf("Error %s\n",ex.what());
	}
}

void test()
{
	GitCtl git;
	git.Open("C:\\tmp\\testrepo\\");
	auto x = git.GetStatus();
	int a = 1;
}