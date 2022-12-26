#pragma once

#include "Debut/Rendering/Shader.h"
#include <glm/glm.hpp>

typedef unsigned int GLenum;

namespace Debut
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filePath);
		OpenGLShader(const std::string& name, const std::string& vertSource, const std::string& fragSource);
		virtual ~OpenGLShader();

		void Bind() const override;
		void Unbind() const override;

		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetBool(const std::string& name, bool value) override;
		virtual void SetIntArray(const std::string& name, int* data, uint32_t count) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& uniform) override;
		virtual void SetFloat(const std::string& name, float uniform) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& uniform) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& uniform) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& uniform) override;

		const std::string& GetName() const override { return m_Name; }
		std::vector<ShaderUniform> GetUniforms() const override;

		void UploadUniformMat3(const std::string& name, const glm::mat3& mat);
		void UploadUniformMat4(const std::string& name, const glm::mat4& mat);

		void UploadUniformFloat(const std::string& name, float val);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& vec);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& vec);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vec);

		void UploadUniformBool(const std::string& name, bool val);
		void UploadUniformInt(const std::string& name, int val);
		void UploadUniformInt2(const std::string& name, const glm::ivec2& vec);
		void UploadUniformInt3(const std::string& name, const glm::ivec3& vec);
		void UploadUniformInt4(const std::string& name, const glm::ivec4& vec);
		void UploadUniformIntArray(const std::string& name, int* data, uint32_t count);

	private:
		std::string ReadFile(const std::string& path);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& src);

		void Compile(std::unordered_map<GLenum, std::string>& sources);
		void Link();

		void CheckCompileError(unsigned int shader);
		void CheckLinkingError(unsigned int program);

	private:
		unsigned int m_ProgramID;
		std::string m_Name;
	};
}
