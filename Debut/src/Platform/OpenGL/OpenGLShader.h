#pragma once

#include "Debut/Renderer/Shader.h"
#include <glm/glm.hpp>

namespace Debut
{
	class OpenGLShader : public Shader
	{
	public:
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
		void CheckCompileError(unsigned int shader);
		void CheckLinkingError(unsigned int program, unsigned int vert, unsigned int frag, const std::string& programName);

	private:
		unsigned int m_ProgramID;
	};
}
