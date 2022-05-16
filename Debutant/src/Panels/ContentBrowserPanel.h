#pragma once

#include <string>
#include <unordered_map>

#include "PropertiesPanel.h"
#include <Debut/Scene/Scene.h>
#include <Debut/Core/Core.h>
#include <Debut/Renderer/Texture.h>
#include <filesystem>
#include "PropertiesPanel.h"

using namespace Debut;

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

	private:
		std::filesystem::path m_CurrDirectory;
		std::unordered_map<std::string, std::string> m_Icons;

		PropertiesPanel* m_PropertiesPanel;
	};
}

