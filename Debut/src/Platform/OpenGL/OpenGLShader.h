#pragma once

#include "Debut/Renderer/Shader.h"
#include <glm/glm.hpp>

typedef unsigned int GLenum;

namespace Debut
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filePath);
		OpenGLShader(const std::string& vertSource, const std::string& fragSource);
		virtual ~OpenGLShader();

		void Bind() const override;
		void Unbind() const override;

		void UploadUniformMat3(const std::string& name, const glm::mat3& mat);
		void UploadUniformMat4(const std::string& name, const glm::mat4& mat);

		void UploadUniformFloat(const std::string& name, float val);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& vec);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& vec);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& vec);

		void UploadUniformInt(const std::string& name, int val);
		void UploadUniformInt2(const std::string& name, const glm::ivec2& vec);
		void UploadUniformInt3(const std::string& name, const glm::ivec3& vec);
		void UploadUniformInt4(const std::string& name, const glm::ivec4& vec);

	private:
		std::string ReadFile(const std::string& path);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& src);

		void Compile(std::unordered_map<GLenum, std::string>& sources);
		void Link();

		void CheckCompileError(unsigned int shader);
		void CheckLinkingError(unsigned int program);

	private:
		unsigned int m_ProgramID;
	};
}
