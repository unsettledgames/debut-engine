#pragma once
#include <string>

namespace Debut
{
	namespace CppUtils
	{
		namespace String
		{
			static bool endsWith(const std::string& str, const std::string& suffix)
			{
				return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
			}

			static bool startsWith(const std::string& str, const std::string& prefix)
			{
				return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
			}
		}
	}
}