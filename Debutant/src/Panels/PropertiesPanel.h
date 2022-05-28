#pragma once

#include <imgui.h>
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
		void DrawName();

		void DrawTextureProperties();
		void DrawShaderProperties();
		void DrawMaterialProperties();

		void DrawPhysicsMaterial2DProperties();

	private:
		std::filesystem::path m_AssetPath = "";
	};
}
