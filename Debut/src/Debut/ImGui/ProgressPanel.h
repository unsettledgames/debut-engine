#pragma once

#include <string>
#include <ctime>
#include <unordered_map>


/*
	TODO:
		- Split name (event key) and label ("Importing x...")
		- Add time measurement (it's enough to save the submission timestamp)
*/

namespace Debut
{
	struct ProgressTask
	{
		std::string Helper;
		uint32_t StartTime;
		float CurrProgress;

		ProgressTask(std::string helper) : Helper(helper)
		{
			StartTime = std::time(nullptr);
			CurrProgress = 0;
		}

		ProgressTask() : Helper(""), StartTime(0), CurrProgress(0) {}
	};

	class ProgressPanel
	{
	public:
		static void OnImGuiRender();

		static void SubmitTask(std::string id, std::string helper);

		static void ProgressTask(std::string task, float increment);
		static void SetProgress(std::string task, float percentage);

		static void CompleteTask(std::string task);

	private:
		static std::unordered_map<std::string, Debut::ProgressTask> s_CurrentTasks;
	};
}