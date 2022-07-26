#pragma once
#include <string>
#include <cstdio>
#include <filesystem>

namespace Debut
{
	namespace CppUtils
	{
		namespace FileSystem
		{
			static std::string CorrectFileName(std::string name)
			{
				char forbidden[] = { ':', '\\', '/', '#', '£', '<', '>', '*', '$', '+', '%', '!', '`',
					'&', '\'', '|', '{', '}', '?', '\"', '=', ' ', '@' };
				for (uint32_t i=0; i<sizeof(forbidden); i++)
					name.erase(std::remove(name.begin(), name.end(), forbidden[i]), name.end());

				return name;
			}

			static bool RemoveFile(std::filesystem::path path)
			{
				if (std::filesystem::exists(path))
					return std::filesystem::remove_all(path);
				return false;
			}
		}

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