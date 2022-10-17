#include "ContentBrowserPanel.h"
#include <Debut/dbtpch.h>

#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Resources/Skybox.h>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/Physics/PhysicsMaterial3D.h>
#include <Debut/AssetManager/AssetManager.h>
#include "Utils/EditorCache.h"
#include <Debut/Utils/CppUtils.h>
#include <Debut/ImGui/ImGuiUtils.h>
#include <Debut/Core/Input.h>
#include <Debut/Core/KeyCodes.h>
#include <imgui.h>
#include <yaml-cpp/yaml.h>

/*
	TODO
		- Move files in content browser
		- Regex for filename selection
*/

namespace Debut
{
	// TODO: change this when we have projects
	static const std::filesystem::path s_AssetsPath = "assets";
	ContentBrowserSettings ContentBrowserPanel::s_Settings;

	ContentBrowserPanel::ContentBrowserPanel()
	{
		EditorCache::Textures().Put("cb-menu", Texture2D::Create("assets/icons/menu.png"));
		EditorCache::Textures().Put("cb-back", Texture2D::Create("assets/icons/back.png"));
		EditorCache::Textures().Put("cb-search", Texture2D::Create("assets/icons/search.png"));
		m_SelectedDir = "assets";

		// Get contents: this is used to show folder first and then files
		for (auto& dirEntry : std::filesystem::directory_iterator(s_AssetsPath))
		{
			// Don't show meta files
			if (dirEntry.path().extension() == ".meta")
				continue;

			// Sort the entry
			if (dirEntry.is_directory())
				m_Dirs.push_back(dirEntry);
			else
				m_Files.push_back(dirEntry);
		}
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content browser");
		
		static float padding = s_Settings.padding;
		static float iconSize = s_Settings.IconSize;
		float cellSize = iconSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);

		if (columnCount < 1)
			columnCount = 1;

		DrawTopBar();		

		// Draw contents 
		for (auto& path : m_Dirs)
		{
			DrawHierarchy(path, true);
			ImGui::NextColumn();
		}

		for (auto& path : m_Files)
		{
			DrawHierarchy(path, false);
			ImGui::NextColumn();
		}

		bool deletePopup = false;
		std::string assetName = "";
		static std::string assetType = "";
		std::string defaultName = "";

		// Can't open popups directly in this menu. What we do instead is save state about what the user
		// chose to do and then open the right popups.
		if (ImGui::BeginPopupContextWindow("##contentbrowsercontext"))
		{
			if (ImGui::BeginMenu("Create..."))
			{
				if (ImGui::MenuItem("Physics Material 2D"))
				{
					assetType = "PhysicsMaterial2D";
					defaultName = "New Physics Material 2D";
				}
				if (ImGui::MenuItem("Phyiscs Material 3D"))
				{
					assetType = "PhysicsMaterial3D";
					defaultName = "New Physics Material 3D";
				}
				if (ImGui::MenuItem("Material"))
				{
					assetType = "Material";
					defaultName = "New Material";
				}
				if (ImGui::MenuItem("Skybox"))
				{
					assetType = "Skybox";
					defaultName = "New Skybox";
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Delete"))
				deletePopup = true;

			ImGui::EndPopup();
		}

		// Delete file popup (thanks ImGui)
		if (deletePopup || ImGui::IsPopupOpen("Delete?") || (ImGui::IsKeyPressed(ImGuiKey_Delete) && ImGui::IsWindowFocused()))
			CBDeleteFile(m_RightClicked);

		// Set asset filename popup (thanks ImGui)
		if (assetType.compare("") != 0 || ImGui::IsPopupOpen("Filename?"))
			assetName = CBChooseFileName(assetType, defaultName);

		// Open asset creation popup
		if (assetType.compare("") != 0 && assetName.compare("") != 0)
		{
			// Skip if the user canceled the action
			if (assetName.compare("!") == 0)
				assetType = "";

			if (assetType.compare("PhysicsMaterial2D") == 0)
				AssetManager::CreateAsset<PhysicsMaterial2D>(m_SelectedDir + "\\" + assetName + ".physmat2d");
			if (assetType.compare("PhysicsMaterial3D") == 0)
				AssetManager::CreateAsset<PhysicsMaterial3D>(m_SelectedDir + "\\" + assetName + ".physmat3d");
			else if (assetType.compare("Material") == 0)
				AssetManager::CreateAsset<Material>(m_SelectedDir + "\\" + assetName + ".mat");
			else if (assetType.compare("Skybox") == 0)
				AssetManager::CreateAsset<Skybox>(m_SelectedDir + "\\" + assetName + ".skybox");

			assetType = "";
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

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		if (!isDir)
			flags |= ImGuiTreeNodeFlags_Leaf;
		if (m_SelectedAsset == path.string())
			flags |= ImGuiTreeNodeFlags_Selected;
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 20, 20 });
		bool treeNodeRet = ImGui::TreeNodeEx((ImGuiUtils::GetFileImguiIcon(isDir ? ".dir" : path.extension().string()) +
			"  " + path.filename().string()).c_str(), flags);
		ImGui::PopStyleVar();

		if (ImGui::IsItemClicked(1))
		{
			m_RightClicked = path.string();
			if (isDir)
				m_SelectedDir = path.string();
			else
			{
				m_SelectedAsset = path.string();
				m_SelectedDir = path.parent_path().string();
			}
		}

		AddDragSource(path);
		
		if (treeNodeRet)
		{
			if (ImGui::IsItemClicked() && isDir)
			{
				m_SelectedDir = path.string();
				m_RightClicked = m_SelectedDir;
			}
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
			{
				m_PropertiesPanel->SetAsset(path);
				m_SelectedAsset = path.string();
				m_RightClicked = m_SelectedAsset;
			}

			// Recursively render the rest of the hierarchy
			if (isDir)
			{
				std::vector<std::filesystem::path> dirs, files;
				// Get contents: this is used to show folder first and then files
				for (auto& dirEntry : std::filesystem::directory_iterator(path))
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

				for (auto& dir : dirs)
					DrawHierarchy(dir, true);
				for (auto& file : files)
					DrawHierarchy(file, false);
			}
			
			ImGui::TreePop();
		}
	}

	void ContentBrowserPanel::DrawModelHierarchy(const Ref<Model>& model)
	{
		static std::unordered_map<UUID, MaterialMetadata> materialDataCache;
		static std::unordered_map<UUID, MeshMetadata> meshDataCache;

		DBT_PROFILE_FUNCTION();
		if (model == nullptr)
			return;

		std::string path = std::filesystem::path(model->GetPath()).filename().string();
		if (path == "")
			path = "Submodel";

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		if (m_RightClicked == model->GetPath())
			flags |= ImGuiTreeNodeFlags_Selected;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 20, 20 });
		bool treeNodeRet = ImGui::TreeNodeEx((ImGuiUtils::GetFileImguiIcon(".model") + "  " + path).c_str(), flags);
		ImGui::PopStyleVar();

		if (ImGui::IsItemClicked())
		{
			m_PropertiesPanel->SetAsset(model->GetPath());
			std::stringstream ss;
			ss << model->GetPath() << model->GetID();
			m_SelectedAsset = ss.str();

			m_RightClicked = model->GetPath();
		}

		if (ImGui::IsItemClicked(1))
		{
			m_RightClicked = path;
			m_SelectedAsset = path;
		}

		AddDragSource(model->GetPath());

		if (treeNodeRet)
		{
			// Recursively render submodels
			for (Debut::UUID model : model->GetSubmodels())
				DrawModelHierarchy(AssetManager::Request<Model>(model));

			// Meshes
			for (Debut::UUID mesh : model->GetMeshes())
			{
				// Get metadata
				MeshMetadata meshData;
				if (meshDataCache.find(mesh) != meshDataCache.end())
					meshData = meshDataCache[mesh];
				else
				{
					meshData = Mesh::GetMetadata(mesh);
					meshDataCache[mesh] = meshData;
				}

				// Format name
				std::stringstream ss;
				ss << mesh;
				std::stringstream meshIDStr;
				meshIDStr << meshData.Name << ".mesh##" << mesh;

				// Render treenode
				ImGuiTreeNodeFlags meshFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding
					| ImGuiTreeNodeFlags_Leaf;
				if (meshIDStr.str() == m_SelectedAsset)
					meshFlags |= ImGuiTreeNodeFlags_Selected;

				if (ImGui::TreeNodeEx((ImGuiUtils::GetFileImguiIcon(".mesh") + "  " + meshIDStr.str()).c_str(), meshFlags))
				{
					if (ImGui::IsItemClicked())
					{
						m_PropertiesPanel->SetAsset(AssetManager::s_AssetsDir + ss.str(), AssetManager::s_MetadataDir + ss.str() + ".meta", AssetType::Mesh);
						m_SelectedAsset = meshIDStr.str();
					}
					ImGui::TreePop();
				}

				AddDragSource(ss.str() + ".mesh");
			}

			// Materials
			for (Debut::UUID material : model->GetMaterials())
			{
				DBT_PROFILE_SCOPE("ContentBrowser::RenderMaterial");

				// Get metadata
				MaterialMetadata materialData;
				if (materialDataCache.find(material) != materialDataCache.end())
					materialData = materialDataCache[material];
				else
				{
					materialData = Material::GetMetadata(material);
					materialDataCache[material] = materialData;
				}

				// Setup name
				std::stringstream ss;
				ss << material;
				std::stringstream matIDStr;
				matIDStr << materialData.Name << ".mat##" << material;

				// Render treenode
				ImGuiTreeNodeFlags matFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding
					| ImGuiTreeNodeFlags_Leaf;
				if (matIDStr.str() == m_SelectedAsset)
					matFlags |= ImGuiTreeNodeFlags_Selected;

				if (ImGui::TreeNodeEx((ImGuiUtils::GetFileImguiIcon(".mat") + "  " + matIDStr.str()).c_str(), matFlags))
				{
					if (ImGui::IsItemClicked())
					{
						m_PropertiesPanel->SetAsset(AssetManager::s_AssetsDir + ss.str(), 
							AssetManager::s_MetadataDir + ss.str() + ".meta", AssetType::Material);
						m_SelectedAsset = matIDStr.str();
					}

					ImGui::TreePop();
				}

				AddDragSource(ss.str() + ".mat");
			}

			ImGui::TreePop();
		}
	}

	void ContentBrowserPanel::DrawTopBar()
	{
		ImGui::BeginGroup();
		// Layout menu
		float iconHeight = ImGui::GetTextLineHeight();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

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

	void ContentBrowserPanel::CBDeleteFile(const std::string& path)
	{
		ImVec2 buttonSize = { 100, ImGui::GetTextLineHeight() * 1.5f };
		ImGui::OpenPopup("Delete?");
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			ImGuiUtils::BoldText("Are you sure you want to delete the file " + m_RightClicked + "?");
				
			ImGuiUtils::VerticalSpace(2);

			ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2.0f - 100);
			if (ImGui::Button("Yes", buttonSize))
			{
				if (std::filesystem::path(path).extension().string() == ".model")
				{
					std::vector<Debut::UUID> ids;
					DeleteModel(AssetManager::Request<Model>(path), ids);
					AssetManager::DeleteAssociations(ids);
				}
				else
				{
					CppUtils::FileSystem::RemoveFile(path);
					std::ifstream inFile(path + ".meta");
					if (inFile.good())
					{
						std::stringstream ss;
						ss << inFile.rdbuf();
						inFile.close();
						YAML::Node node = YAML::Load(ss.str());

						CppUtils::FileSystem::RemoveFile(path + ".meta");
						std::vector<Debut::UUID> id; id.push_back(node["ID"].as<uint64_t>());
						AssetManager::DeleteAssociations({ id });
					}
				}

				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No", buttonSize))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}

	std::string ContentBrowserPanel::CBChooseFileName(const std::string& type, const std::string& defaultFilename)
	{
		ImVec2 buttonSize = { 150, ImGui::GetTextLineHeight() * 1.5f };
		ImGui::OpenPopup("Filename?");
		ImGui::SetNextWindowSize({ 600, 150 });
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Filename?", NULL))
		{
			ImGuiUtils::BoldText("Specify a name for the " + type + ".");

			ImGuiUtils::VerticalSpace(2);

			static char fileName[128];
			memcpy(fileName, defaultFilename.c_str(), defaultFilename.length());
			
			// Filename text input
			ImGuiUtils::StartColumns(2, { 100, 500 });
			ImGui::Text("File name: ");
			ImGuiUtils::NextColumn();
			ImGui::SetNextItemWidth(500);
			bool set = ImGui::InputText("##filename:", fileName, 128, ImGuiInputTextFlags_EnterReturnsTrue);
			ImGuiUtils::ResetColumns();

			ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2.0f - 150);
			if (ImGui::Button("Create", buttonSize) || set)
			{
				std::string ret(fileName);
				memset(fileName, 0, 128);

				ImGui::CloseCurrentPopup(); 
				ImGui::EndPopup();
				
				return ret;
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", buttonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				memset(fileName, 0, 128);
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				
				return "!";
			}

			ImGui::EndPopup();
		}

		return "";
	}

	void ContentBrowserPanel::DeleteModel(Ref<Model> model, std::vector<Debut::UUID> deletedIds)
	{
		deletedIds.push_back(model->GetID());
		CppUtils::FileSystem::RemoveFile(model->GetPath());
		CppUtils::FileSystem::RemoveFile(model->GetPath() + ".meta");

		for (uint32_t i = 0; i < model->GetMeshes().size(); i++)
		{
			std::stringstream ss;
			// Delete meshes
			ss << AssetManager::s_AssetsDir << model->GetMeshes()[i];
			CppUtils::FileSystem::RemoveFile(ss.str());
			ss.str("");

			ss << AssetManager::s_MetadataDir << model->GetMeshes()[i] << ".meta";
			CppUtils::FileSystem::RemoveFile(ss.str());
			ss.str("");

			deletedIds.push_back(model->GetMeshes()[i]);

			// Delete materials
			ss << AssetManager::s_AssetsDir << model->GetMaterials()[i];
			CppUtils::FileSystem::RemoveFile(ss.str());
			ss.str("");

			ss << AssetManager::s_MetadataDir << model->GetMaterials()[i] << ".meta";
			CppUtils::FileSystem::RemoveFile(ss.str());
			ss.str("");

			deletedIds.push_back(model->GetMaterials()[i]);
		}

		for (uint32_t i = 0; i < model->GetSubmodels().size(); i++)
			DeleteModel(AssetManager::Request<Model>(model->GetSubmodels()[i]), deletedIds);
	}

	void ContentBrowserPanel::AddDragSource(const std::filesystem::path& path)
	{
		if (ImGui::BeginDragDropSource())
		{
			const wchar_t* itemPath = path.c_str();
			ImGui::SetDragDropPayload("CONTENT_BROWSER_DATA", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
			ImGui::EndDragDropSource();
		}
	}
}