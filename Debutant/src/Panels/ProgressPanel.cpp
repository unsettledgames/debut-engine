#include "ProgressPanel.h"
#include <imgui.h>

namespace Debut
{
	std::unordered_map<std::string, float> ProgressPanel::s_CurrentTasks;

	void ProgressPanel::OnImGuiRender()
	{
		float barHeight = 35;
		bool open = false;
		if (s_CurrentTasks.size() > 0)
			open = true;


		ImGui::Begin("Tasks", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar 
			| ImGuiWindowFlags_Modal);

		for (auto& task : s_CurrentTasks)
		{
			// Print task data
			ImGui::Text(task.first.c_str());
			ImGui::ProgressBar(task.second, { 500, barHeight });
		}

		ImGui::End();
	}

	void ProgressPanel::SubmitTask(std::string name)
	{
		s_CurrentTasks[name] = 0;
	}

	void ProgressPanel::ProgressTask(std::string task, float increment)
	{
		s_CurrentTasks[task] += increment;
	}

	void ProgressPanel::SetProgress(std::string task, float percentage)
	{
		s_CurrentTasks[task] = percentage;
	}

	void ProgressPanel::CompleteTask(std::string task)
	{
		s_CurrentTasks.erase(task);
	}
}