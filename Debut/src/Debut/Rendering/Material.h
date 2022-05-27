#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/Texture.h>

#include <unordered_map>

namespace Debut
{
	class Material
	{
	public:
		void SetFloat(const std::string& name, float val);
		void SetVec2(const std::string& name, const glm::vec2& vec);
		void SetVec3(const std::string& name, const glm::vec3& vec);
		void SetVec4(const std::string& name, const glm::vec4& vec);

		void SetInt(const std::string& name, int val);
		void SetTexture(const Ref<Texture2D> texture);

	private:
		std::string m_Path;
		std::string m_Name;
		std::unordered_map<std::string, ShaderUniform> m_Uniforms;
	};
}