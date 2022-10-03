#pragma once
#include <string>
#include <cstdio>
#include <filesystem>
#include <stack>

namespace Debut
{
	namespace CppUtils
	{
		namespace FileSystem
		{
			static inline std::string CorrectFileName(std::string name)
			{
				char forbidden[] = { ':', '\\', '/', '#', '£', '<', '>', '*', '$', '+', '%', '!', '`',
					'&', '\'', '|', '{', '}', '?', '\"', '=', ' ', '@' };
				for (uint32_t i=0; i<sizeof(forbidden); i++)
					name.erase(std::remove(name.begin(), name.end(), forbidden[i]), name.end());

				return name;
			}

			static inline bool RemoveFile(std::filesystem::path path)
			{
				if (std::filesystem::exists(path))
				{
					if (std::filesystem::is_directory(path))
						return std::filesystem::remove_all(path);
					else
						return std::filesystem::remove(path);
				}
					
				return false;
			}

			static std::vector<std::string> GetAllFilesWithExtension(const std::string& extension, const std::string& folder)
			{
				std::vector<std::string> ret;
				std::stack<std::filesystem::path> pathsToVisit;
				pathsToVisit.push(std::filesystem::path(folder));
				std::filesystem::path currPath;

				while (pathsToVisit.size() > 0)
				{
					currPath = pathsToVisit.top();
					pathsToVisit.pop();

					auto& dirIt = std::filesystem::directory_iterator(currPath);

					for (auto entry : dirIt)
					{
						if (entry.is_directory())
							pathsToVisit.push(entry.path());
						else
						{
							std::string fileExt = entry.path().extension().string();
							if (fileExt.compare(extension) == 0)
								ret.push_back(entry.path().string());
						}
					}
				}

				return ret;
			}
		}

		namespace String
		{
			static inline bool EndsWith(const std::string& str, const std::string& suffix)
			{
				return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
			}

			static inline bool StartsWith(const std::string& str, const std::string& prefix)
			{
				return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
			}
		}
	}
}