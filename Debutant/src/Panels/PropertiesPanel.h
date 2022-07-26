#pragma once

#include <imgui.h>
#include <filesystem>

namespace Debut
{
	enum class AssetType
	{
		None, Mesh, Material, Texture2D, Shader, PhysicsMaterial2D, Model
	};

	class PropertiesPanel
	{
	public:
		PropertiesPanel() = default;

		void OnImGuiRender();

		void SetAsset(std::filesystem::path path, AssetType type = AssetType::None);

		std::filesystem::path GetAsset() { return m_AssetPath; }

	private:
		void DrawName();

		void DrawTextureProperties();
		void DrawShaderProperties();
		void DrawMaterialProperties();
		void DrawModelProperties();

		void DrawPhysicsMaterial2DProperties();

	private:
		std::filesystem::path m_AssetPath = "";
		AssetType m_AssetType = AssetType::None;
	};
}
