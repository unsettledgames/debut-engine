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
		std::ifstream skyboxFile(path);
		std::ifstream metaFile(path + ".meta");

		std::stringstream ss;
		ss << skyboxFile.rdbuf();
		YAML::Node skyboxData = YAML::Load(ss.str());

		ss.str("");
		ss << metaFile.rdbuf();
		YAML::Node metaData = YAML::Load(ss.str());

		UUID frontId = skyboxData["FrontTexture"].as<uint64_t>();
		UUID bottomId = skyboxData["BottomTexture"].as<uint64_t>();
		UUID leftId = skyboxData["LeftTexture"].as<uint64_t>();
		UUID rightId = skyboxData["RightTexture"].as<uint64_t>();
		UUID upId = skyboxData["UpTexture"].as<uint64_t>();
		UUID downId = skyboxData["DownTexture"].as<uint64_t>();

		if (frontId && bottomId && leftId && rightId && upId && downId)
		{
			switch (Renderer::GetAPI())
			{
			case RendererAPI::API::OpenGL:
				ret = CreateRef<OpenGLSkybox>(AssetManager::GetPath(frontId), AssetManager::GetPath(bottomId),
					AssetManager::GetPath(leftId), AssetManager::GetPath(rightId), AssetManager::GetPath(upId),
					AssetManager::GetPath(downId));
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
			ret->m_Material = skyboxData["Material"].as<uint64_t>();
			ret->m_ID = metaData["ID"].as<uint64_t>();
			ret->m_Path = path;
			ret->m_Name = (std::filesystem::path(path)).filename().string();

			ret->m_Textures["Front"] = frontId;
			ret->m_Textures["Bottom"] = bottomId;
			ret->m_Textures["Left"] = leftId;
			ret->m_Textures["Right"] = rightId;
			ret->m_Textures["Up"] = upId;
			ret->m_Textures["Down"] = downId;

			return ret;
		}
	}

	void Skybox::SaveDefaultConfig(const std::string& path)
	{
		SkyboxConfig config = { 0, 0, 0, 0, 0, 0, 0, UUID()};
		Skybox::SaveSettings(config, path);
	}

	void Skybox::SaveSettings(SkyboxConfig config, const std::string& path)
	{
		std::ofstream skyboxFile(path);
		std::ofstream metaFile(path + ".meta");

		YAML::Emitter skyboxEmitter;
		YAML::Emitter metaEmitter;

		skyboxEmitter << YAML::BeginDoc << YAML::BeginMap;

		skyboxEmitter << YAML::Key << "Material" << YAML::Value << config.Material;
		skyboxEmitter << YAML::Key << "FrontTexture" << YAML::Value << config.FrontTexture;
		skyboxEmitter << YAML::Key << "BottomTexture" << YAML::Value << config.BottomTexture;
		skyboxEmitter << YAML::Key << "LeftTexture" << YAML::Value << config.LeftTexture;
		skyboxEmitter << YAML::Key << "RightTexture" << YAML::Value << config.RightTexture;
		skyboxEmitter << YAML::Key << "UpTexture" << YAML::Value << config.UpTexture;
		skyboxEmitter << YAML::Key << "DownTexture" << YAML::Value << config.DownTexture;

		skyboxEmitter << YAML::EndMap << YAML::EndDoc;

		metaEmitter << YAML::BeginDoc << YAML::BeginMap;
		metaEmitter << YAML::Key << "ID" << YAML::Value << config.ID;
		metaEmitter << YAML::EndMap << YAML::EndDoc;

		skyboxFile << skyboxEmitter.c_str();
		metaFile << metaEmitter.c_str();
	}
}