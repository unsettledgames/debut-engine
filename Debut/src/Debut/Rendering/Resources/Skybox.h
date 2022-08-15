#include <Debut/Core/Core.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Shader.h>

#pragma once

namespace Debut
{
	struct SkyboxConfig
	{
		UUID FrontTexture;
		UUID BottomTexture;
		UUID LeftTexture;
		UUID RightTexture;
		UUID UpTexture;
		UUID DownTexture;

		UUID Material;
	};
	
	class Skybox
	{
	public:
		Skybox() {}
		Skybox(const Skybox&) {}
		virtual ~Skybox() = default;

		virtual void Bind() {}
		virtual void Unbind() {}

		virtual uint32_t GetRendererID() { return m_ID; }

		static Ref<Skybox> Create(const std::string& path);

		static void SaveDefaultConfig(const std::string& path);
		static void SaveSettings(SkyboxConfig config, const std::string& path);

		inline void SetMaterial(UUID material) { m_Material = material; }

		inline UUID GetTexture(const std::string& type) { return m_Textures[type]; }
		inline UUID GetMaterial() { return m_Material; }
		inline Mesh& GetMesh() { return m_Mesh; }
		inline UUID GetID() { return m_ID; }
		inline std::string GetName() { return m_Name; }
		inline std::string GetPath() { return m_Path; }

	protected:
		UUID m_Material = 0;
		UUID m_ID = 0;
		Mesh m_Mesh;

		std::string m_Name;
		std::string m_Path;

		std::unordered_map<std::string, UUID> m_Textures;
	};
}