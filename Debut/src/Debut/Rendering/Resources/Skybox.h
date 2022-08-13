#include <Debut/Core/Core.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Shader.h>

#pragma once

namespace Debut
{
	class Skybox
	{
	public:
		virtual ~Skybox() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual uint32_t GetRendererID() = 0;

		static Ref<Skybox> Create(const std::string& front, const std::string& bottom, const std::string& left,
			const std::string& right, const std::string& up, const std::string& down, Ref<Shader> shader);

		Material& GetMaterial() { return m_Material; }
		Mesh& GetMesh() { return m_Mesh; }

	protected:
		Material m_Material;
		Mesh m_Mesh;
	};
}