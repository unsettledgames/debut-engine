#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/Texture.h>

#include <unordered_map>

namespace Debut
{
	struct MaterialConfig
	{
		std::string Name;
		UUID Shader;
		std::vector<ShaderUniform> Uniforms;
	};

	class Material
	{
	public:
		Material() = default;
		Material(const std::string& path);

		void Use(const glm::mat4& cameraTransform);
		void Unuse();

		void SetShader(Ref<Shader> shader);
		void SetName(const std::string& name) { m_Name = name; }
		void SetPath(const std::string& path) { m_Path = path; }

		void SetConfig(const MaterialConfig& config);
		void SaveSettings();
		static void SaveSettings(const std::string& path, const MaterialConfig& config);
		static void SaveDefaultConfig(const std::string& path);

		void SetFloat(const std::string& name, float val);
		void SetVec2(const std::string& name, const glm::vec2& vec);
		void SetVec3(const std::string& name, const glm::vec3& vec);
		void SetVec4(const std::string& name, const glm::vec4& vec);

		void SetInt(const std::string& name, int val);
		void SetBool(const std::string& name, bool val);
		void SetTexture(const std::string& name, const Ref<Texture2D> texture);

		UUID GetID() { return m_ID; }
		UUID GetShader() { return m_Shader; }
		std::string GetName() { return m_Name; }
		std::string GetPath() { return m_Path; }
		std::vector<ShaderUniform> GetUniforms();

	private:
		UUID m_ID;
		UUID m_Shader;

		std::string m_Path;
		std::string m_Name;
		std::unordered_map<std::string, ShaderUniform> m_Uniforms;
	};
}