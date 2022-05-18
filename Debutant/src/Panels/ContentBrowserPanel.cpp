#include "ContentBrowserPanel.h"

#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/AssetManager/AssetManager.h>
#include "Utils/EditorCache.h"
#include <imgui.h>

using namespace Debut;

namespace Debutant
{
	// TODO: change this when we have projects
	static const std::filesystem::path s_AssetsPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel() : m_CurrDirectory(s_AssetsPath)
	{
		EditorCache::Textures().Put("cb-genericfile", Texture2D::Create("assets/icons/file.png"));
		EditorCache::Textures().Put("cb-directory", Texture2D::Create("assets/icons/directory.png"));

		m_Icons["txt"] = "cb-genericfile";
		m_Icons["dir"] = "cb-directory";
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content browser");

		// TODO: Right click menu
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Physics Material 2D"))
				AssetManager::CreateAsset<PhysicsMaterial2D>(m_CurrDirectory.string() + "/NewPhysicsMaterial2D.physmat2d");
			ImGui::EndPopup();
		}
		
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
			// Don't show meta files
			if (path.extension().string() == ".meta")
				continue;

			std::string pathName = path.string();

			auto relativePath = std::filesystem::relative(path, s_AssetsPath);
			std::string relativePathString = relativePath.string();

			Ref<Texture2D> icon = dirEntry.is_directory() ? EditorCache::Textures().Get("cb-directory") : GetFileIcon(dirEntry.path());

			ImGui::PushID(pathName.c_str());

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { iconSize, iconSize }, { 0, 1 }, { 1, 0 }))
			{
				m_PropertiesPanel->SetAsset(path);
			}
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
	Ref<Texture2D> ContentBrowserPanel::GetFileIcon(const std::filesystem::path& path)
	{
		std::string extension = path.extension().string();

		if (extension == ".png")
		{
			Ref<Texture2D> texture = EditorCache::Textures().Get(path.string());
			if (!texture)
			{
				texture = Texture2D::Create(path.string());
				EditorCache::Textures().Put(path.string(), texture);
			}

			return texture;
		}

		// Search for the right icon, if the extension isn't supported, return a generic file icon
		auto& tex = m_Icons.find(extension);
		if (tex == m_Icons.end())
			return EditorCache::Textures().Get("cb-genericfile");

		return EditorCache::Textures().Get(tex->second);
	}
}