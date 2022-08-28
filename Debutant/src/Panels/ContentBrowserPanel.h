#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "PropertiesPanel.h"
#include <Debut/Scene/Scene.h>
#include <Debut/Core/Core.h>
#include <Debut/Rendering/Resources/Model.h>
#include <Debut/Rendering/Texture.h>
#include <filesystem>
#include "PropertiesPanel.h"

#define MAX_NAME_LENGTH	128

namespace Debut
{
	struct ContentBrowserSettings
	{
		float IconSize = 64.0;
		float padding = 4.0;
	};

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

		void SetPropertiesPanel(PropertiesPanel* panel) { m_PropertiesPanel = panel; }

	private:
		Ref<Texture2D> GetFileIcon(const std::filesystem::path& path);

		void DrawTopBar();
		void DrawHierarchy(const std::filesystem::path& path, bool isDir);
		void DrawModelHierarchy(const Ref<Model>& model);

		void AddDragSource(const std::filesystem::path& path);

		void CBDeleteFile(const std::string& path);
		void DeleteModel(Ref<Model> model, std::vector<Debut::UUID> deletedIds);

	private:
		static ContentBrowserSettings s_Settings;
		bool m_LayoutMenuOpen = false;

		std::unordered_set<std::string> m_OpenDirs;
		std::unordered_map<std::string, std::string> m_Icons;

		std::string m_SelectedAsset;
		std::string m_SelectedDir;
		std::string m_RightClicked;

		char m_RenameBuffer[MAX_NAME_LENGTH];
		bool m_IsRenaming = false;

		PropertiesPanel* m_PropertiesPanel;
	};
}

