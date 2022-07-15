#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "PropertiesPanel.h"
#include <Debut/Scene/Scene.h>
#include <Debut/Core/Core.h>
#include <Debut/Rendering/Texture.h>
#include <filesystem>
#include "PropertiesPanel.h"

#define MAX_NAME_LENGTH	128
using namespace Debut;

// TODO: unload textures from the cache

/*
	The main things to do are 2:
DONE	1 - Column-based layout instead of grid-based with icons
		2 - Model importing and properties

DONE	1 - Have a dropdown menu at the top left corner of the browser
DONE		- Let the user choose between different layouts
DONE		- Depending on the current layout, render the content browser differently
		2 - When the user clicks on a model source file:
			- If the .model object exists, render data in the properties panel
			- If it doesn't, render an import window in the properties panel
				- The user can choose which importing settings to use:
					- Normal generation
					- Tangent space generation
					- Model optimization
				- When the user presses the import button, a progress bar appears
*/

namespace Debutant
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

		void DrawEntry(const std::filesystem::path& path, bool isDirectory);
		void DrawTopBar();
		void DrawHierarchy(std::filesystem::path path, bool isDir);

		void AddDragSource(const std::filesystem::path path);

	private:
		static ContentBrowserSettings s_Settings;
		bool m_LayoutMenuOpen = false;

		std::filesystem::path m_CurrDirectory;
		std::unordered_set<std::string> m_OpenDirs;
		std::unordered_map<std::string, std::string> m_Icons;

		std::string m_SelectedAsset;

		char m_RenameBuffer[MAX_NAME_LENGTH];
		bool m_IsRenaming = false;

		PropertiesPanel* m_PropertiesPanel;
	};
}

