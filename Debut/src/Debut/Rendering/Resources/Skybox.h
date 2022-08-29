#include <Debut/Core/Core.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <unordered_map>

#pragma once

namespace Debut
{
	enum class SkyboxTexture : uint8_t { Front, Bottom, Up, Down, Left, Right };

	struct SkyboxConfig
	{
		std::unordered_map<SkyboxTexture, UUID> Textures;

		UUID Material;
		UUID ID;
	};
	
	class Skybox
	{
	public:
		Skybox() {}
		Skybox(const Skybox&) {}
		virtual ~Skybox() = default;

		virtual void Bind() {}
		virtual void Unbind() {}

		virtual uint32_t GetRendererID() const { return 0; }

		static Ref<Skybox> Create(const std::string& path);

		static void SaveDefaultConfig(const std::string& path);
		static void SaveSettings(SkyboxConfig config, const std::string& path);
		static SkyboxConfig GetConfig(const std::string& path);

		inline void SetMaterial(UUID material) { m_Material = material; }

		inline UUID GetTexture(SkyboxTexture type) { return m_Textures[type]; }
		inline UUID GetMaterial() { return m_Material; }
		inline Mesh& GetMesh() { return m_Mesh; }
		virtual inline UUID GetID() { return m_ID; }
		inline std::string GetName() { return m_Name; }
		inline std::string GetPath() { return m_Path; }

		virtual void Reload() = 0;

	protected:
		UUID m_Material = 0;
		UUID m_ID = 0;
		Mesh m_Mesh;

		std::string m_Name;
		std::string m_Path;

		std::unordered_map<SkyboxTexture, UUID> m_Textures;
	};
}