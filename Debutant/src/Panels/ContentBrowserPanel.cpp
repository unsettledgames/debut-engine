#include "ContentBrowserPanel.h"

#include <Debut/Rendering/Material.h>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/AssetManager/AssetManager.h>
#include "Utils/EditorCache.h"
#include <Debut/ImGui/ImGuiUtils.h>
#include <imgui.h>

using namespace Debut;

namespace Debutant
{
	// TODO: change this when we have projects
	static const std::filesystem::path s_AssetsPath = "assets";
	ContentBrowserSettings ContentBrowserPanel::s_Settings;

	ContentBrowserPanel::ContentBrowserPanel() : m_CurrDirectory(s_AssetsPath)
	{
		EditorCache::Textures().Put("cb-genericfile", Texture2D::Create("assets/icons/file.png"));
		EditorCache::Textures().Put("cb-directory", Texture2D::Create("assets/icons/directory.png"));
		EditorCache::Textures().Put("cb-menu", Texture2D::Create("assets/icons/menu.png"));
		EditorCache::Textures().Put("cb-back", Texture2D::Create("assets/icons/back.png"));
		EditorCache::Textures().Put("cb-search", Texture2D::Create("assets/icons/search.png"));

		m_Icons["txt"] = "cb-genericfile";
		m_Icons["dir"] = "cb-directory";
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		std::vector<std::filesystem::path> dirs, files;
		ImGui::Begin("Content browser");

		// Right click menu
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create new Physics Material 2D"))
				AssetManager::CreateAsset<PhysicsMaterial2D>(m_CurrDirectory.string() + "/NewPhysicsMaterial2D.physmat2d");
			if (ImGui::MenuItem("Create new Material"))
				AssetManager::CreateAsset<Material>(m_CurrDirectory.string() + "/NewMaterial.mat");
			ImGui::EndPopup();
		}
		
		static float padding = s_Settings.padding;
		static float iconSize = s_Settings.IconSize;
		float cellSize = iconSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);

		if (columnCount < 1)
			columnCount = 1;

		DrawTopBar();

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		// Get contents: this is used to show folder first and then files
		for (auto& dirEntry : std::filesystem::directory_iterator(m_CurrDirectory))
		{
			// Don't show meta files
			if (dirEntry.path().extension() == ".meta")
				continue;

			// Sort the entry
			if (dirEntry.is_directory())
				dirs.push_back(dirEntry);
			else
				files.push_back(dirEntry);
		}

		// Draw contents 
		for (auto& path : dirs)
		{
			DrawHierarchy(path, true);
			ImGui::NextColumn();
		}

		for (auto& path : files)
		{
			DrawHierarchy(path, false);
			ImGui::NextColumn();
		}

		ImGui::End();
	}

	void ContentBrowserPanel::DrawHierarchy(const std::filesystem::path& path, bool isDir)
	{
		if (path.extension() == ".model")
		{
			DrawModelHierarchy(AssetManager::Request<Model>(path.string()));
			return;
		}

		Ref<Texture2D> icon = isDir ? EditorCache::Textures().Get("cb-directory") : GetFileIcon(path);
		auto openFolder = std::find(m_OpenDirs.begin(), m_OpenDirs.end(), path.filename().string());
		bool folderOpen = openFolder != m_OpenDirs.end() || path.string() == m_PropertiesPanel->GetAsset().string();
		bool treeNodeClicked = ImGuiUtils::ImageTreeNode(path.filename().string().c_str(), (ImTextureID)icon->GetRendererID(), folderOpen);

		if (treeNodeClicked || folderOpen)
		{
			// Toggle the current node if it's been clicked
			if (isDir)
				if (folderOpen && treeNodeClicked)
					m_OpenDirs.erase(openFolder);
				else
					m_OpenDirs.insert(path.filename().string());
			else
				m_PropertiesPanel->SetAsset(path);

			// Recursively render the rest of the hierarchy
			if (isDir)
			{
				for (auto& dirEntry : std::filesystem::directory_iterator(path))
				{
					const std::filesystem::path& otherPath = dirEntry.path();
					// Don't show meta files
					if (otherPath.extension().string() == ".meta")
						continue;

					DrawHierarchy(otherPath, dirEntry.is_directory());
				}
			}
			
			ImGui::TreePop();
		}

		AddDragSource(path);
	}

	void ContentBrowserPanel::DrawModelHierarchy(const Ref<Model>& model)
	{
		Ref<Texture2D> modelIcon = GetFileIcon(".model");
		std::string path = std::filesystem::path(model->GetPath()).filename().string();
		auto openFolder = std::find(m_OpenDirs.begin(), m_OpenDirs.end(), model->GetPath());
		bool folderOpen = openFolder != m_OpenDirs.end() || model->GetPath() == m_PropertiesPanel->GetAsset().string();
		if (path == "")
			path = "Submodel";

		bool treeNodeClicked = ImGuiUtils::ImageTreeNode(path.c_str(), (ImTextureID)modelIcon->GetRendererID(), folderOpen);

		if (treeNodeClicked || folderOpen)
		{
			// Toggle the current node if it's been clicked
			if (folderOpen && treeNodeClicked)
			{
				m_OpenDirs.erase(openFolder);
				m_PropertiesPanel->SetAsset(model->GetPath());
			}
			else
				m_OpenDirs.insert(model->GetPath());
				

			// Recursively render submodels
			for (Debut::UUID model : model->GetSubmodels())
				DrawModelHierarchy(AssetManager::Request<Model>(model));

			// Meshes
			// Materials

			ImGui::TreePop();
		}

		std::stringstream ss;
		ss << model->GetID();
		AddDragSource(model->GetPath());
	}

	void ContentBrowserPanel::DrawTopBar()
	{
		ImGui::BeginGroup();
		// Layout menu
		float iconHeight = ImGui::GetTextLineHeight();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		if (ImGui::ImageButton((ImTextureID)EditorCache::Textures().Get("cb-back")->GetRendererID(),
			{ ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() }, { 0, 1 }, { 1, 0 }) && m_CurrDirectory != s_AssetsPath)
			m_CurrDirectory = m_CurrDirectory.parent_path();

		// Pop 0 item spacing
		ImGui::PopStyleVar();

		ImGui::SameLine();

		// Search bar
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 2, 2 });
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetTextLineHeight() - 4);
		char searchBar[256] = "";
		ImGui::InputText("##Searchbar", searchBar, 256);
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		// Search bar icon
		ImGui::SameLine();
		ImGui::ImageButton((ImTextureID)EditorCache::Textures().Get("cb-search")->GetRendererID(),
			{ iconHeight ,iconHeight }, { 0, 1 }, { 1, 0 });
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

		ImGui::Dummy({ 0, 4 });
		ImGui::Separator();

		ImGui::EndGroup();
	}

	void ContentBrowserPanel::AddDragSource(const std::filesystem::path& path)
	{
		if (ImGui::BeginDragDropSource())
		{
			Log.CoreInfo("Dragging {0}", path.string());
			const wchar_t* itemPath = path.c_str();
			ImGui::SetDragDropPayload("CONTENT_BROWSER_DATA", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
			ImGui::EndDragDropSource();
		}
	}

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