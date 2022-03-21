#include "Debut/dbtpch.h"
#include "OpenGLShader.h"
#include "Debut/Core/Log.h"
#include "glm/gtc/type_ptr.hpp"
#include <array>
#include <glad/glad.h>

namespace Debut
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex") return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;

		DBT_CORE_ASSERT(false, "Unknown shader type");
		return -1;
	}

	OpenGLShader::OpenGLShader(const std::string& filePath)
	{
		std::string src = ReadFile(filePath);
		auto shaderSources = PreProcess(src);

		Compile(shaderSources);
		Link();

		// Extract shader name from the file path
		auto last = filePath.find_last_of("/\\");
		auto dot = filePath.rfind(".");
		last = last == std::string::npos ? last : last + 1;
		auto count = dot == std::string::npos ? filePath.size() - last : dot - last;

		m_Name = filePath.substr(last, count);
	}

	void OpenGLShader::Compile(std::unordered_map<GLenum, std::string>& sources)
	{
		DBT_CORE_ASSERT(sources.size() <= 3, "Only 3 shaders per file are supported at the moment.");

		GLuint program = glCreateProgram();
		std::array<GLenum, 3> shaderIDs;
		int shaderIdx = 0;

		// Compile each shader and attach it to the program
		for (auto& kv : sources)
		{
			GLenum type = kv.first;
			const std::string& src = kv.second;
			const GLchar* glSource = src.c_str();

			GLuint shader = glCreateShader(type);
			glShaderSource(shader, 1, &glSource, nullptr);
			glCompileShader(shader);

#ifdef DBT_DEBUG
			CheckCompileError(shader);
#endif
			glAttachShader(program, shader);
			shaderIDs[shaderIdx++] = shader;
		}

		m_ProgramID = program;

		// Clear shaders
		for (GLenum id : shaderIDs)
			glDeleteShader(id);
	}

	void OpenGLShader::Link()
	{
		glLinkProgram(m_ProgramID);

#ifdef DBT_DEBUG
		CheckLinkingError(m_ProgramID);
#endif
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& src)
	{
		std::unordered_map<GLenum, std::string> ret;
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = src.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			size_t eol = src.find_first_of("\r\n", pos);
			DBT_CORE_ASSERT(eol != std::string::npos, "Shader syntax error");
			size_t begin = pos + typeTokenLength + 1;
			
			std::string type = src.substr(begin, eol - begin);
			DBT_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel", "Invalid or unsupported shader type");
			size_t nextLinePos = src.find_first_not_of("\r\n", eol);
			pos = src.find(typeToken, nextLinePos);

			ret[ShaderTypeFromString(type)] = src.substr(nextLinePos, pos - (nextLinePos == std::string::npos ?
				src.size() - 1 : nextLinePos));
		}

		return ret;
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertSource, const std::string& fragSource)
	{
		m_Name = name;
		std::unordered_map<GLenum, std::string> shaderSources;

		shaderSources[GL_VERTEX_SHADER] = vertSource;
		shaderSources[GL_FRAGMENT_SHADER] = fragSource;

		Compile(shaderSources);
		Link();
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_ProgramID);
	}

	std::string OpenGLShader::ReadFile(const std::string& path)
	{
		std::string fileContent;
		std::ifstream inFile(path, std::ios::in | std::ios::binary);

		if (inFile)
		{
			inFile.seekg(0, std::ios::end);
			fileContent.resize(inFile.tellg());
			inFile.seekg(0, std::ios::beg);

			inFile.read(&fileContent[0], fileContent.size());
			inFile.close();
		}
		else
			DBT_CORE_ERROR("Couldn't open file %s", path.c_str());

		return fileContent;
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_ProgramID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& uniform)
	{
		UploadUniformMat4(name, uniform);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& uniform)
	{
		UploadUniformFloat3(name, uniform);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& uniform)
	{
		UploadUniformFloat4(name, uniform);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& mat)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& mat)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float val)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform1f(location, val);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& vec)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform2f(location, vec.x, vec.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& vec)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform3f(location, vec.x, vec.y, vec.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vec)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
	}
	

	void OpenGLShader::UploadUniformInt(const std::string& name, int val)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform1i(location, val);
	}

	void OpenGLShader::UploadUniformInt2(const std::string& name, const glm::ivec2& vec)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform2i(location, vec.x, vec.y);
	}

	void OpenGLShader::UploadUniformInt3(const std::string& name, const glm::ivec3& vec)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform3i(location, vec.x, vec.y, vec.z);
	}

	void OpenGLShader::UploadUniformInt4(const std::string& name, const glm::ivec4& vec)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform4i(location, vec.x, vec.y, vec.z, vec.w);
	}

	void OpenGLShader::CheckCompileError(unsigned int shader)
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

	void OpenGLShader::CheckLinkingError(unsigned int program)
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

			// Use the infoLog as you see fit.
			Log.CoreError("Shader program failed to link: %s", infoLog.data());
			DBT_ASSERT(false, "Program failed to link");

			// In this simple program, we'll just leave
			return;
		}
	}
}