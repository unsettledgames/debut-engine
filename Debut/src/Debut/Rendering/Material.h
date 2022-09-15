#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>
#include <Debut/Rendering/Shader.h>

#include <unordered_map>

/*
	I changed the Material class! What do I do?
	- Update DrawMaterialProperties in PropertiesPanel
	- Serialize / deserialize
	- Add ShaderUniform if necessary
*/

namespace Debut
{
	struct MaterialConfig
	{
		std::string Name;
		UUID ID;

		UUID Shader;
		std::unordered_map<std::string, ShaderUniform> Uniforms;
	};

	struct MaterialMetadata
	{
		std::string Name;
		UUID ID;
	};

	class Skybox;
	class Texture2D;

	class Material
	{
		friend class Renderer3D;
	public:
		Material() = default;
		Material(const std::string& path, const std::string& metaPath);

		void Use();
		void Unuse();

		void SetShader(Ref<Shader> shader);
		void SetUniforms(std::unordered_map<std::string, ShaderUniform> uniforms) { m_Uniforms = uniforms; }
		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetPath(const std::string& path) { m_Path = path; }
		inline void SetMetaPath(const std::string& metaPath) { m_MetaPath = metaPath; }

		void SetConfig(const MaterialConfig& config);
		void SaveSettings();
		static void SaveSettings(const std::string& path, const MaterialConfig& config);
		static void SaveDefaultConfig(const std::string& path);

		void SetFloat(const std::string& name, float val);
		void SetVec2(const std::string& name, const glm::vec2& vec);
		void SetVec3(const std::string& name, const glm::vec3& vec);
		void SetVec4(const std::string& name, const glm::vec4& vec);

		void SetMat4(const std::string& name, const glm::mat4& mat);

		void SetInt(const std::string& name, int val);
		void SetBool(const std::string& name, bool val);
		void SetTexture(const std::string& name, const Ref<Texture2D> texture);
		void SetCubemap(const std::string& name, const Ref<Skybox> texture);

		inline UUID GetID() { return m_ID; }
		inline UUID GetShader() { return m_Shader; }
		inline std::string GetName() { return m_Name; }
		inline std::string GetPath() { return m_Path; }
		inline std::unordered_map<std::string, ShaderUniform> GetUniforms() { return m_Uniforms; }
		inline bool IsValid() { return m_Valid; }

		void Reload();
		static MaterialMetadata GetMetadata(UUID id);
		static std::vector<std::string> GetDefaultUniforms() { return s_DefaultUniforms; }

	private:
		void Load(std::ifstream& file);

	private:
		UUID m_ID;
		bool m_Valid;

		UUID m_Shader;
		static UUID s_PrevShader;
		std::unordered_map<std::string, ShaderUniform> m_Uniforms;

		std::string m_Path;
		std::string m_MetaPath;
		std::string m_Name;

		static std::vector<std::string> s_DefaultUniforms;
	};
}