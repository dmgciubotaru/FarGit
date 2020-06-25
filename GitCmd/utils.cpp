#include "utils.h"

#include <algorithm>

std::string ToUpper(const std::string& str)
{
	std::string output;
	std::transform(str.begin(), str.end(), std::back_inserter(output), [](unsigned char c) { return std::toupper(c); });
	return output;
}