#include "ContentBrowserPanel.h"

#include <Debut/Rendering/Material.h>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/AssetManager/AssetManager.h>
#include "Utils/EditorCache.h"
#include <Debut/Utils/CppUtils.h>
#include <Debut/ImGui/ImGuiUtils.h>
#include <Debut/Core/Input.h>
#include <Debut/Core/KeyCodes.h>
#include <imgui.h>

using namespace Debut;

/*
	TODO
		- Delete file:
			- Ask for confirmation and just delete it if it's a normal file
			- Also delete related files if it's a model
		- Create file in selected directory
		- Move files in content browser
*/

namespace Debutant
{
	// TODO: change this when we have projects
	static const std::filesystem::path s_AssetsPath = "assets";
	ContentBrowserSettings ContentBrowserPanel::s_Settings;

	ContentBrowserPanel::ContentBrowserPanel()
	{
		EditorCache::Textures().Put("cb-genericfile", Texture2D::Create("assets/icons/file.png"));
		EditorCache::Textures().Put("cb-directory", Texture2D::Create("assets/icons/directory.png"));
		EditorCache::Textures().Put("cb-menu", Texture2D::Create("assets/icons/menu.png"));
		EditorCache::Textures().Put("cb-back", Texture2D::Create("assets/icons/back.png"));
		EditorCache::Textures().Put("cb-search", Texture2D::Create("assets/icons/search.png"));
		EditorCache::Textures().Put("cb-model", Texture2D::Create("assets/icons/model.png"));
		EditorCache::Textures().Put("cb-mesh", Texture2D::Create("assets/icons/mesh.png"));
		EditorCache::Textures().Put("cb-material", Texture2D::Create("assets/icons/material.png"));
		EditorCache::Textures().Put("cb-unimported-model", Texture2D::Create("assets/icons/unimported_model.png"));

		m_Icons[".txt"] = "cb-genericfile";
		m_Icons["dir"] = "cb-directory";
		m_Icons[".model"] = "cb-model";
		m_Icons[".mesh"] = "cb-mesh";
		m_Icons[".mat"] = "cb-material";
		m_Icons[".obj"] = "cb-unimported-model";
		m_Icons[".fbx"] = "cb-unimported-model";
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		std::vector<std::filesystem::path> dirs, files;
		ImGui::Begin("Content browser");
		
		static float padding = s_Settings.padding;
		static float iconSize = s_Settings.IconSize;
		float cellSize = iconSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);

		if (columnCount < 1)
			columnCount = 1;

		DrawTopBar();

		// Get contents: this is used to show folder first and then files
		for (auto& dirEntry : std::filesystem::directory_iterator(s_AssetsPath))
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

		bool deletePopup = false;

		if (ImGui::BeginPopupContextWindow("##contentbrowsercontext"))
		{
			if (ImGui::BeginMenu("Create..."))
			{
				if (ImGui::MenuItem("Create new Physics Material 2D"))
					AssetManager::CreateAsset<PhysicsMaterial2D>(m_SelectedDir + "\\NewPhysicsMaterial2D.physmat2d");
				if (ImGui::MenuItem("Create new Material"))
					AssetManager::CreateAsset<Material>(m_SelectedDir + "\\NewMaterial.mat");

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Delete"))
				deletePopup = true;

			ImGui::EndPopup();
		}
		if (deletePopup || ImGui::IsPopupOpen("Delete?") || (ImGui::IsKeyPressed(ImGuiKey_Delete) && ImGui::IsWindowFocused()))
			CBDeleteFile(m_RightClicked);

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
				m_SelectedAsset = path.string();
		}

		AddDragSource(path);
		
		if (treeNodeRet)
		{
			if (ImGui::IsItemClicked())
			{
				// Toggle the current node if it's been clicked
				if (isDir)
				{
					m_SelectedDir = path.string();
					m_RightClicked = m_SelectedDir;
				}
				else
				{
					m_PropertiesPanel->SetAsset(path);
					m_SelectedAsset = path.string();
					m_RightClicked = m_SelectedAsset;
				}
			}

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
	}

	void ContentBrowserPanel::DrawModelHierarchy(const Ref<Model>& model)
	{
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
				auto meshData = Mesh::GetMetadata(mesh);
				std::stringstream ss;
				ss << mesh;
				std::stringstream meshIDStr;
				meshIDStr << meshData.Name << ".mesh##" << mesh;

				ImGuiTreeNodeFlags meshFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding
					| ImGuiTreeNodeFlags_Leaf;

				if (meshIDStr.str() == m_SelectedAsset)
					meshFlags |= ImGuiTreeNodeFlags_Selected;

				if (ImGui::TreeNodeEx((ImGuiUtils::GetFileImguiIcon(".mesh") + "  " + meshIDStr.str()).c_str(), meshFlags))
				{
					if (ImGui::IsItemClicked())
					{
						m_PropertiesPanel->SetAsset(ss.str(), AssetType::Mesh);
						m_SelectedAsset = meshIDStr.str();
					}
					ImGui::TreePop();
				}

				AddDragSource(ss.str() + ".mesh");
			}

			// Materials
			for (Debut::UUID material : model->GetMaterials())
			{
				auto materialData = Material::GetMetadata(material);
				std::stringstream ss;
				ss << material;
				std::stringstream matIDStr;
				matIDStr << materialData.Name << ".mat##" << material;
				ImGuiTreeNodeFlags matFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding
					| ImGuiTreeNodeFlags_Leaf;

				if (matIDStr.str() == m_SelectedAsset)
					matFlags |= ImGuiTreeNodeFlags_Selected;

				if (ImGui::TreeNodeEx((ImGuiUtils::GetFileImguiIcon(".mat") + "  " + matIDStr.str()).c_str(), matFlags))
				{
					if (ImGui::IsItemClicked())
					{
						m_PropertiesPanel->SetAsset(ss.str(), AssetType::Material);
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
		bool open = true;
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			ImGui::Text(("Are you sure you want to delete the file " + m_RightClicked + "?").c_str());
				
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