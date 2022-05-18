#include "Debut/dbtpch.h"
#include "Renderer.h"
#include "Texture.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "yaml-cpp/yaml.h"
#include <fstream>

namespace Debut
{
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		std::ifstream metaFile(path + ".meta");
		std::stringstream strStream;

		Texture2DConfig texParams = { Texture2DParameter::FILTERING_LINEAR, Texture2DParameter::WRAP_CLAMP };

		if (metaFile.good())
		{
			strStream << metaFile.rdbuf();
			YAML::Node in = YAML::Load(strStream.str().c_str());

			texParams.Filtering = StringToTex2DParam(in["Filtering"].as<std::string>());
			texParams.WrapMode = StringToTex2DParam(in["WrapMode"].as<std::string>());
			texParams.ID = in["ID"] ? in["ID"].as<uint64_t>() : 0;
		}

		// TODO: textures are created by path: this is a good opportunity to load the parameters from 
		// the meta file.
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
}