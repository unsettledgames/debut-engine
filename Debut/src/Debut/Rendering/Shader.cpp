#pragma once

#include <Debut/dbtpch.h>
#include <Debut/Core/Log.h>
#include <Debut/Rendering/Renderer/Renderer.h>
#include <Debut/Rendering/Shader.h>
#include <Platform/OpenGL/OpenGLShader.h>
#include <yaml-cpp/yaml.h>

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
		return nullptr;
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
		return nullptr;
	}

	void Shader::CreateOrLoadMeta(const std::string& path)
	{
		std::ifstream meta(path + ".meta");
		std::stringstream ss;

		if (meta.good())
		{
			// Load data
			ss << meta.rdbuf();
			YAML::Node node = YAML::Load(ss.str());

			m_ID = node["ID"].as<uint64_t>();
		}
		else
		{
			// Automatically create a meta file
			meta.close();

			std::ofstream newMeta(path + ".meta");
			YAML::Emitter emitter;

			emitter << YAML::BeginMap << YAML::Key << "ID" << YAML::Value << m_ID << YAML::EndMap;

			newMeta << emitter.c_str();
			newMeta.close();
		}
	}
}
