#include "Debut/dbtpch.h"
#include "Renderer/Renderer.h"
#include "Texture.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "yaml-cpp/yaml.h"
#include <fstream>

namespace Debut
{
	Ref<Texture2D> Texture2D::Create(const std::string& path, const std::string& metaFilePath)
	{
		std::string correctMeta = metaFilePath;
		if (correctMeta == "")
			correctMeta = path + ".meta";
		std::stringstream strStream;

		// Load texture parameters
		Texture2DConfig texParams = GetConfig(correctMeta);
		// If the texture doesn't have a meta file, save some default ones
		if (texParams.ID == 0)
		{
			texParams.ID = UUID();
			SaveSettings(texParams, path);
		}

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(path, texParams);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(width, height);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
		return nullptr;
	}

	void Texture2D::SaveDefaultConfig(const std::string& path)
	{
	}

	void Texture2D::SaveSettings(const Texture2DConfig& parameters, const std::string& path)
	{
		YAML::Emitter emitter;

		emitter << YAML::BeginDoc << YAML::BeginMap;

		emitter << YAML::Key << "Asset" << YAML::Value << "Texture2D";
		emitter << YAML::Key << "ID" << YAML::Value << parameters.ID;
		emitter << YAML::Key << "Filtering" << YAML::Value << Tex2DParamToString(parameters.Filtering);
		emitter << YAML::Key << "WrapMode" << YAML::Value << Tex2DParamToString(parameters.WrapMode);

		emitter << YAML::EndMap << YAML::EndDoc;

		std::ofstream outFile(path + ".meta");
		outFile << emitter.c_str();
		outFile.close();
	}

	Texture2DConfig Texture2D::GetConfig(const std::string& path)
	{
		std::ifstream metaFile(path);
		std::stringstream strStream;
		Texture2DConfig texParams;

		if (metaFile.good())
		{
			strStream << metaFile.rdbuf();
			YAML::Node in = YAML::Load(strStream.str().c_str());

			texParams.Filtering = StringToTex2DParam(in["Filtering"].as<std::string>());
			texParams.WrapMode = StringToTex2DParam(in["WrapMode"].as<std::string>());
			texParams.ID = in["ID"] ? in["ID"].as<uint64_t>() : 0;

			return texParams;
		}
		else
			return { Texture2DParameter::FILTERING_LINEAR, Texture2DParameter::WRAP_CLAMP, 0 };
		
	}
}