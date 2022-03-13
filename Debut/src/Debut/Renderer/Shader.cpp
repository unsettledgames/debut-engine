#pragma once

#include <Debut/dbtpch.h>
#include <Debut/Log.h>
#include <Debut/Renderer/Shader.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Debut
{
	Shader::Shader(const std::string& vertSource, const std::string& fragSource)
	{
		unsigned int vertShader = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);

		const char* srcVert = vertSource.c_str();
		const char* srcFrag = fragSource.c_str();

		glShaderSource(vertShader, 1, &srcVert, nullptr);
		glShaderSource(fragShader, 1, &srcFrag, nullptr);

		glCompileShader(vertShader);
		glCompileShader(fragShader);

#ifdef DBT_DEBUG
		CheckCompileError(vertShader);
		CheckCompileError(fragShader);
#endif

		m_ProgramID = glCreateProgram();

		glAttachShader(m_ProgramID, vertShader);
		glAttachShader(m_ProgramID, fragShader);

		glLinkProgram(m_ProgramID);

#ifdef DBT_DEBUG
		CheckLinkingError(m_ProgramID, vertShader, fragShader, "default");
#endif
		// Clean everything
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_ProgramID);
	}

	void Shader::Bind() const
	{
		glUseProgram(m_ProgramID);
	}

	void Shader::Unbind() const
	{
		glUseProgram(0);
	}

	void Shader::UploadUniformMat4(const std::string& name, const glm::mat4& mat)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
	}

	void Shader::CheckCompileError(unsigned int shader)
	{
		GLint isCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

			// Provide the infolog in whatever manor you deem best.
			Log.CoreError("%s", errorLog.data());
			DBT_CORE_ASSERT(false, "Shader failed to compile");
			// Exit with failure.
			glDeleteShader(shader); // Don't leak the shader.
			return;
		}
	}

	void Shader::CheckLinkingError(unsigned int program, unsigned int vert, unsigned int frag, const std::string& programName)
	{
		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak shaders either.
			glDeleteShader(vert);
			glDeleteShader(frag);

			// Use the infoLog as you see fit.
			Log.CoreError("%s", programName, infoLog.data());
			DBT_ASSERT(false, "Program failed to link");

			// In this simple program, we'll just leave
			return;
		}
	}
}
