#pragma once

#include <filesystem>

namespace Debut
{
	class PropertiesPanel
	{
	public:
		PropertiesPanel() = default;

		void OnImGuiRender();

		void SetAsset(std::filesystem::path path);

	private:
		void DrawTextureProperties();
		void DrawPhysicsMaterial2DProperties();

	private:
		std::filesystem::path m_AssetPath = "";
	};
}
