#pragma once

#include <Debut/dbtpch.h>
#include <Debut/Core/Log.h>
#include <Debut/Renderer/Renderer.h>
#include <Debut/Renderer/Shader.h>
#include <Platform/OpenGL/OpenGLShader.h>

namespace Debut
{
	Ref<Shader> Shader::Create(const std::string& filePath)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLShader>(filePath);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertSrc, const std::string& fragSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLShader>(name, vertSrc, fragSrc);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		DBT_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end(), "Shader is already in the library");
		m_Shaders[name] = shader;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader, const std::string& name)
	{
		DBT_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end(), "Shader is already in the library");
		m_Shaders[name] = shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& filePath)
	{
		auto shader = Shader::Create(filePath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filePath)
	{
		auto shader = Shader::Create(filePath);
		Add(shader, name);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		DBT_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end(), "Shader to get has not been found");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name) const
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}
}
