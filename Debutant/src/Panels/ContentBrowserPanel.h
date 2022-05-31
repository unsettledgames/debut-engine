#pragma once

#include <string>
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

namespace Debutant
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();
		void SetPropertiesPanel(PropertiesPanel* panel) { m_PropertiesPanel = panel; }

	private:
		Ref<Texture2D> GetFileIcon(const std::filesystem::path& path);
		void DrawEntry(const std::filesystem::path& path, bool isDirectory);

	private:
		std::filesystem::path m_CurrDirectory;
		std::unordered_map<std::string, std::string> m_Icons;

		std::string m_SelectedAsset;

		char m_RenameBuffer[MAX_NAME_LENGTH];
		bool m_IsRenaming = false;

		PropertiesPanel* m_PropertiesPanel;
	};
}

