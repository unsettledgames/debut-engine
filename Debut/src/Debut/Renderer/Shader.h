#pragma once

#include <Debut/Core.h>
#include <glm/glm.hpp>

namespace Debut
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static Ref<Shader> Create(const std::string& vertSrc, const std::string& fragSrc);
		static Ref<Shader> Create(const std::string& filePath);

	protected:
		unsigned int m_ProgramID;
	};
}