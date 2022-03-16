#pragma once

#include <glm/glm.hpp>

namespace Debut
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static Shader* Create(const std::string& vertSrc, const std::string& fragSrc);

	private:
		void CheckCompileError(unsigned int shader);
		void CheckLinkingError(unsigned int program, unsigned int vert, unsigned int frag, const std::string& programName);

	protected:
		unsigned int m_ProgramID;
	};
}