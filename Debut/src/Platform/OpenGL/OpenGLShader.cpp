#include "Debut/dbtpch.h"
#include "OpenGLShader.h"
#include "OpenGLError.h"
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

	static ShaderDataType GLToDbtUniformType(GLenum type)
	{
		switch (type)
		{
		case GL_FLOAT: return ShaderDataType::Float;
		case GL_FLOAT_VEC2: return ShaderDataType::Float2;
		case GL_FLOAT_VEC3: return ShaderDataType::Float3;
		case GL_FLOAT_VEC4: return ShaderDataType::Float4;

		case GL_INT: return ShaderDataType::Int;
		case GL_INT_VEC2: return ShaderDataType::Int2;
		case GL_INT_VEC3: return ShaderDataType::Int3;
		case GL_INT_VEC4: return ShaderDataType::Int4;

		case GL_BOOL: return ShaderDataType::Bool;
		case GL_SAMPLER_2D: return ShaderDataType::Sampler2D;
		case GL_SAMPLER_CUBE: return ShaderDataType::SamplerCube;
		case GL_FLOAT_MAT3: return ShaderDataType::Mat3;
		case GL_FLOAT_MAT4: return ShaderDataType::Mat4;
		}

		DBT_ASSERT(false, "Unsupported uniform type {0}", type);
		return ShaderDataType::None;
	}

	OpenGLShader::OpenGLShader(const std::string& filePath)
	{
		DBT_PROFILE_FUNCTION();
		CreateOrLoadMeta(filePath);

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
		DBT_PROFILE_FUNCTION();
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
		DBT_PROFILE_FUNCTION();
		glLinkProgram(m_ProgramID);

#ifdef DBT_DEBUG
		CheckLinkingError(m_ProgramID);
#endif
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& src)
	{
		DBT_PROFILE_FUNCTION();
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
		DBT_PROFILE_FUNCTION();
		m_Name = name;
		std::unordered_map<GLenum, std::string> shaderSources;

		shaderSources[GL_VERTEX_SHADER] = vertSource;
		shaderSources[GL_FRAGMENT_SHADER] = fragSource;

		Compile(shaderSources);
		Link();
	}

	OpenGLShader::~OpenGLShader()
	{
		DBT_PROFILE_FUNCTION();
		glDeleteProgram(m_ProgramID);
	}

	std::string OpenGLShader::ReadFile(const std::string& path)
	{
		DBT_PROFILE_FUNCTION();
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
			Log.CoreError("Couldn't open file {0}", path.c_str());

		return fileContent;
	}

	void OpenGLShader::Bind() const
	{
		DBT_PROFILE_FUNCTION();
		GLCall(glUseProgram(m_ProgramID));
	}

	void OpenGLShader::Unbind() const
	{
		GLCall(glUseProgram(0));
	}

	std::vector<ShaderUniform> OpenGLShader::GetUniforms() const
	{
		std::vector<ShaderUniform> ret;

		GLint i;
		GLint count;

		GLint size; // size of the variable
		GLenum type; // type of the variable (float, vec3 or mat4, etc)

		const GLsizei bufSize = 128; // maximum name length
		GLchar name[bufSize]; // variable name in GLSL
		GLsizei length; // name length

		GLCall(glGetProgramiv(m_ProgramID, GL_ACTIVE_UNIFORMS, &count));
		ret.resize(count);

		for (i = 0; i < count; i++)
		{
			ShaderUniform::UniformData placeHolder;

			GLCall(glGetActiveUniform(m_ProgramID, (GLuint)i, bufSize, &length, &size, &type, name));
			ret[i] = { name, GLToDbtUniformType(type), placeHolder };
		}

		return ret;
	}

	void OpenGLShader::SetInt(const std::string& name, int uniform)
	{
		UploadUniformInt(name, uniform);
	}

	void OpenGLShader::SetBool(const std::string& name, bool uniform)
	{
		UploadUniformBool(name, uniform);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* data, uint32_t count)
	{
		UploadUniformIntArray(name, data, count);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& uniform)
	{
		UploadUniformMat4(name, uniform);
	}

	void OpenGLShader::SetFloat(const std::string& name, float uniform)
	{
		UploadUniformFloat(name, uniform);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& uniform)
	{
		UploadUniformFloat2(name, uniform);
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
		GLCall(GLuint location = glGetUniformLocation(m_ProgramID, name.c_str()));
		GLCall(glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat)));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& mat)
	{
		GLCall(GLuint location = glGetUniformLocation(m_ProgramID, name.c_str()));
		GLCall(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat)));
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float val)
	{
		GLCall(GLuint location = glGetUniformLocation(m_ProgramID, name.c_str()));
		GLCall(glUniform1f(location, val));
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& vec)
	{
		GLCall(GLuint location = glGetUniformLocation(m_ProgramID, name.c_str()));
		GLCall(glUniform2f(location, vec.x, vec.y));
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& vec)
	{
		GLCall(GLuint location = glGetUniformLocation(m_ProgramID, name.c_str()));
		GLCall(glUniform3f(location, vec.x, vec.y, vec.z));
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vec)
	{
		GLCall(GLuint location = glGetUniformLocation(m_ProgramID, name.c_str()));
		GLCall(glUniform4f(location, vec.x, vec.y, vec.z, vec.w));
	}
	

	void OpenGLShader::UploadUniformBool(const std::string& name, bool val)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform1i(location, val);
	}


	void OpenGLShader::UploadUniformInt(const std::string& name, int val)
	{
		GLuint location = glGetUniformLocation(m_ProgramID, name.c_str());
		glUniform1i(location, val);
	}

	void OpenGLShader::UploadUniformInt2(const std::string& name, const glm::ivec2& vec)
	{
		GLCall(GLuint location = glGetUniformLocation(m_ProgramID, name.c_str()));
		GLCall(glUniform2i(location, vec.x, vec.y));
	}

	void OpenGLShader::UploadUniformInt3(const std::string& name, const glm::ivec3& vec)
	{
		GLCall(GLuint location = glGetUniformLocation(m_ProgramID, name.c_str()));
		GLCall(glUniform3i(location, vec.x, vec.y, vec.z));
	}

	void OpenGLShader::UploadUniformInt4(const std::string& name, const glm::ivec4& vec)
	{
		GLCall(GLuint location = glGetUniformLocation(m_ProgramID, name.c_str()));
		GLCall(glUniform4i(location, vec.x, vec.y, vec.z, vec.w));
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* data, uint32_t count)
	{
		GLCall(GLuint location = glGetUniformLocation(m_ProgramID, name.c_str()));
		GLCall(glUniform1iv(location, count, data));
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
			Log.CoreError("Shader compile error: {0}", errorLog.data());
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
			Log.CoreError("Shader program failed to link: {0}", infoLog.data());
			DBT_ASSERT(false, "Program failed to link");

			// In this simple program, we'll just leave
			return;
		}
	}
}