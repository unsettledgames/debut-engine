#include "ContentBrowserPanel.h"
#include <imgui.h>

namespace Debut
{
	// TODO: change this when we have projects
	static const std::filesystem::path s_AssetsPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel() : m_CurrDirectory(s_AssetsPath) 
	{
		m_Icons.emplace("genericfile", Texture2D::Create("assets/icons/file.png"));
		m_Icons.emplace("directory", Texture2D::Create("assets/icons/directory.png"));
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content browser");

		static float padding = 4.0f;
		static float iconSize = 64.0f;
		float cellSize = iconSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);

		if (columnCount < 1)
			columnCount = 1;

		// Put current path and back button to the top
		if (m_CurrDirectory != s_AssetsPath)
		{
			if (ImGui::Button("Back"))
				m_CurrDirectory = m_CurrDirectory.parent_path();
			ImGui::SameLine();
		}
		ImGui::Text(m_CurrDirectory.string().c_str());

		// Draw contents 
		ImGui::Columns(columnCount, 0, false);
		// TODO: cache this so you don't have to keep accessing the filesystem
		for (auto dirEntry : std::filesystem::directory_iterator(m_CurrDirectory))
		{
			const std::filesystem::path& path = dirEntry.path();
			std::string pathName = path.string();

			auto relativePath = std::filesystem::relative(path, s_AssetsPath);
			std::string relativePathString = relativePath.string();

			Ref<Texture2D> icon = dirEntry.is_directory() ? m_Icons["directory"] : GetFileIcon(dirEntry.path().extension().string());

			ImGui::PushID(pathName.c_str());

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { iconSize, iconSize }, { 0, 1 }, { 1, 0 });
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itemPath = path.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_DATA", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (dirEntry.is_directory())
					m_CurrDirectory /= path.filename();
			}

			ImGui::TextWrapped(relativePath.filename().string().c_str());
			ImGui::PopID();

			ImGui::NextColumn();
		}

		ImGui::End();
	}

	// TODO: show texture preview! Should probably pass the whole path
	Ref<Texture2D> ContentBrowserPanel::GetFileIcon(const std::string& extension)
	{
		// Search for the right icon, if the extension isn't supported, return a generic file icon
		auto& tex = m_Icons.find(extension);
		if (tex == m_Icons.end())
			return m_Icons["genericfile"];
		
		return tex->second;
	}
}