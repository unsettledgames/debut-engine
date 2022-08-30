#include <Debut/dbtpch.h>
#include <Debut/Core/Core.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Resources/Skybox.h>
#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/Texture.h>

#include <Platform/OpenGL/OpenGLSkybox.h>

#include <yaml-cpp/yaml.h>
#include "Debut/Rendering/Renderer/Renderer.h"

namespace Debut
{
	Ref<Skybox> Skybox::Create(const std::string& path)
	{
		Ref<Skybox> ret = nullptr;
		SkyboxConfig config = GetConfig(path);

		if (config.Textures[SkyboxTexture::Front] && config.Textures[SkyboxTexture::Bottom] && 
			config.Textures[SkyboxTexture::Left] && config.Textures[SkyboxTexture::Right] && 
			config.Textures[SkyboxTexture::Up] && config.Textures[SkyboxTexture::Down])
		{
			switch (Renderer::GetAPI())
			{
			case RendererAPI::API::OpenGL:
				ret = CreateRef<OpenGLSkybox>(AssetManager::GetPath(config.Textures[SkyboxTexture::Front]), 
					AssetManager::GetPath(config.Textures[SkyboxTexture::Bottom]), AssetManager::GetPath(config.Textures[SkyboxTexture::Left]), 
					AssetManager::GetPath(config.Textures[SkyboxTexture::Right]), AssetManager::GetPath(config.Textures[SkyboxTexture::Up]),
					AssetManager::GetPath(config.Textures[SkyboxTexture::Down]));
			}
		}
		else
		{
			switch (Renderer::GetAPI())
			{
			case RendererAPI::API::OpenGL:
				ret = CreateRef<OpenGLSkybox>();
			}
		}

		if (ret != nullptr)
		{
			std::vector<float> cubeVertices = { -1, -1,  1,	1, -1, 1,	-1,  1,  1,	1,  1,  1,	-1, -1, -1,	 1, -1, -1,	-1,  1, -1, 1,  1, -1 };
			std::vector<int> cubeIndices = { 2, 6, 7, 2, 3, 7, 0, 4, 5, 0, 1, 5, 0, 2, 6, 0, 4, 6, 1, 3, 7, 1, 5, 7, 0, 2, 3,
				0, 1, 3, 4, 6, 7, 4, 5, 7 };

			ret->m_Mesh.SetPositions(cubeVertices);
			ret->m_Mesh.SetIndices(cubeIndices);

			// Load material from meta file
			ret->m_Material = config.Material;
			ret->m_ID = config.ID;
			ret->m_Path = path;
			ret->m_Name = (std::filesystem::path(path)).filename().string();
			ret->m_Textures = config.Textures;

			return ret;
		}
	}

	void Skybox::SaveDefaultConfig(const std::string& path)
	{
		SkyboxConfig config;
		config.Textures[SkyboxTexture::Front] = 0;
		config.Textures[SkyboxTexture::Bottom] = 0;
		config.Textures[SkyboxTexture::Left] = 0;
		config.Textures[SkyboxTexture::Right] = 0;
		config.Textures[SkyboxTexture::Up] = 0;
		config.Textures[SkyboxTexture::Down] = 0;
		 
		Skybox::SaveSettings(config, path);
	}

	SkyboxConfig Skybox::GetConfig(const std::string& path)
	{
		SkyboxConfig config;

		std::ifstream skyboxFile(path);
		std::stringstream ss;
		ss << skyboxFile.rdbuf();
		YAML::Node skyboxData = YAML::Load(ss.str());

		std::ifstream metaFile(path + ".meta");
		ss.str("");
		ss << metaFile.rdbuf();
		YAML::Node metaData = YAML::Load(ss.str());

		config.Textures[SkyboxTexture::Front] = skyboxData["FrontTexture"].as<uint64_t>();
		config.Textures[SkyboxTexture::Bottom] = skyboxData["BottomTexture"].as<uint64_t>();
		config.Textures[SkyboxTexture::Left] = skyboxData["LeftTexture"].as<uint64_t>();
		config.Textures[SkyboxTexture::Right] = skyboxData["RightTexture"].as<uint64_t>();
		config.Textures[SkyboxTexture::Up] = skyboxData["UpTexture"].as<uint64_t>();
		config.Textures[SkyboxTexture::Down] = skyboxData["DownTexture"].as<uint64_t>();
		config.Material = skyboxData["Material"].as<uint64_t>();
		config.ID = metaData["ID"].as<uint64_t>();

		return config;
	}

	void Skybox::SaveSettings(SkyboxConfig config, const std::string& path)
	{
		std::ofstream skyboxFile(path);
		std::ofstream metaFile(path + ".meta");

		YAML::Emitter skyboxEmitter;
		YAML::Emitter metaEmitter;

		skyboxEmitter << YAML::BeginDoc << YAML::BeginMap;

		skyboxEmitter << YAML::Key << "Material" << YAML::Value << config.Material;
		skyboxEmitter << YAML::Key << "FrontTexture" << YAML::Value << config.Textures[SkyboxTexture::Front];
		skyboxEmitter << YAML::Key << "BottomTexture" << YAML::Value << config.Textures[SkyboxTexture::Bottom];
		skyboxEmitter << YAML::Key << "LeftTexture" << YAML::Value << config.Textures[SkyboxTexture::Left];
		skyboxEmitter << YAML::Key << "RightTexture" << YAML::Value << config.Textures[SkyboxTexture::Right];
		skyboxEmitter << YAML::Key << "UpTexture" << YAML::Value << config.Textures[SkyboxTexture::Up];
		skyboxEmitter << YAML::Key << "DownTexture" << YAML::Value << config.Textures[SkyboxTexture::Down];

		skyboxEmitter << YAML::EndMap << YAML::EndDoc;

		metaEmitter << YAML::BeginDoc << YAML::BeginMap;
		metaEmitter << YAML::Key << "ID" << YAML::Value << config.ID;
		metaEmitter << YAML::EndMap << YAML::EndDoc;

		skyboxFile << skyboxEmitter.c_str();
		metaFile << metaEmitter.c_str();
	}
}