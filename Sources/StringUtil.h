#pragma once

#include <algorithm>
#include <string>

static inline std::string ToUpper(const std::string& s)
{
	std::string upper;
	upper.resize(s.size());
	std::transform(s.begin(), s.end(), upper.begin(), ::toupper);
	return upper;
}