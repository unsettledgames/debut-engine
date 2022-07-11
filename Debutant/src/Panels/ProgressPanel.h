#pragma once

#include <string>
#include <unordered_map>

namespace Debut
{
	class ProgressPanel
	{
	public:
		static void OnImGuiRender();

		static void SubmitTask(std::string name);

		static void ProgressTask(std::string task, float increment);
		static void SetProgress(std::string task, float percentage);

		static void CompleteTask(std::string task);

	private:
		static std::unordered_map<std::string, float> s_CurrentTasks;
	};
}