#pragma once

#include <filesystem>

namespace Debutant
{
	class PropertiesPanel
	{
	public:
		PropertiesPanel() = default;

		void OnImGuiRender();

		void SetAsset(std::filesystem::path path) { m_AssetPath = path; }

	private:
		void DrawTextureProperties();

	private:
		std::filesystem::path m_AssetPath = "";
	};
}
