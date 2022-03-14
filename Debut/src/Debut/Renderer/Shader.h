#pragma once

#include <glm/glm.hpp>

namespace Debut
{
	class Shader
	{
	public:
		Shader(const std::string& vertSource, const std::string& fragSource);
		~Shader();

		void Bind() const;
		void Unbind() const;

		void UploadUniformMat4(const std::string& name, const glm::mat4& mat);

	private:
		void CheckCompileError(unsigned int shader);
		void CheckLinkingError(unsigned int program, unsigned int vert, unsigned int frag, const std::string& programName);

	private:
		unsigned int m_ProgramID;
	};
}