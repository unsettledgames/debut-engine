#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/Texture.h>

#include <unordered_map>

namespace Debut
{
	class Material
	{
	public:
		Material() = default;
		Material(const std::string& path);

		void SetFloat(const std::string& name, float val);
		void SetVec2(const std::string& name, const glm::vec2& vec);
		void SetVec3(const std::string& name, const glm::vec3& vec);
		void SetVec4(const std::string& name, const glm::vec4& vec);

		void SetInt(const std::string& name, int val);
		void SetBool(const std::string& name, bool val);
		void SetTexture(const std::string& name, const Ref<Texture2D> texture);

		UUID GetID() { return m_ID; }

	private:
		UUID m_ID;
		std::string m_Path;
		std::string m_Name;
		std::unordered_map<std::string, ShaderUniform> m_Uniforms;
	};
}