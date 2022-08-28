#include "ProgressPanel.h"
#include <Debut/ImGui/ImGuiUtils.h>
#include <imgui.h>
#include <sstream>

namespace Debut
{
	std::unordered_map<std::string, Debut::ProgressTask> ProgressPanel::s_CurrentTasks;

	void ProgressPanel::OnImGuiRender()
	{
		float barHeight = 35;
		bool open = false;
		if (s_CurrentTasks.size() > 0)
			ImGui::Begin("Tasks", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar 
				| ImGuiWindowFlags_Modal);

		for (auto& task : s_CurrentTasks)
		{
			uint32_t currTime = std::time(nullptr);
			uint32_t elapsed = currTime - task.second.StartTime;

			uint32_t minutes = elapsed / 60;
			uint32_t seconds = elapsed - minutes * 60;

			std::stringstream ss;
			ss << "Time elapsed: " << minutes << "." << seconds;

			// Print task data
			ImGuiUtils::StartColumns(2, { 300, (uint32_t)ImGui::GetContentRegionAvail().x - 300 });
			ImGui::Text(task.second.Helper.c_str());
			ImGui::NextColumn();
			ImGui::Text(ss.str().c_str());
			ImGuiUtils::ResetColumns();

			ImGui::ProgressBar(task.second.CurrProgress, { 500, barHeight });
		}
		if (s_CurrentTasks.size() > 0)
			ImGui::End();
	}

	void ProgressPanel::SubmitTask(std::string name, std::string helper)
	{
		Debut::ProgressTask task(helper);
		s_CurrentTasks[name] = task;
	}

	void ProgressPanel::ProgressTask(std::string task, float increment)
	{
		s_CurrentTasks[task].CurrProgress += increment;
	}

	void ProgressPanel::SetProgress(std::string task, float percentage)
	{
		s_CurrentTasks[task].CurrProgress = percentage;
	}

	void ProgressPanel::CompleteTask(std::string task)
	{
		s_CurrentTasks.erase(task);
	}
}