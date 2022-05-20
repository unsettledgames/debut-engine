#include <Debut/dbtpch.h>
#include <yaml-cpp/yaml.h>
#include <imgui.h>
#include <filesystem>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Physics/PhysicsMaterial2D.h>

namespace Debut
{
	static std::unordered_map<UUID, std::string> s_AssetMap;

	static AssetCache<std::string, Ref<Texture2D>> s_TextureCache;
	static AssetCache<std::string, Ref<PhysicsMaterial2D>> s_PhysicsMaterial2DCache;

	void AssetManager::Init()
	{
		// Check if the database file exists
		std::ifstream mapFile("Debut\\AssetMap.yaml");
		std::stringstream ss;

		// If it exists, load it and have a <ID, path> map loaded in memory
		if (mapFile.good())
		{
			YAML::Node inYaml;

			ss << mapFile.rdbuf();
			inYaml = YAML::Load(ss.str());

			YAML::Node associations = inYaml["Associations"];

			for (uint32_t i=0; i<associations.size(); i++)
			{
				s_AssetMap[associations[i]["ID"].as<uint64_t>()] = associations[i]["Path"].as<std::string>();
			}

			mapFile.close();
		}
		// Create the asset map 
		else
		{
			mapFile.close();

			if (!std::filesystem::exists("Debut"))
				std::filesystem::create_directory("Debut");

			std::ofstream toCreate("Debut\\AssetMap.yaml");
			YAML::Emitter emitter;

			emitter << YAML::BeginMap << YAML::Key << "Associations" << YAML::Value << YAML::BeginSeq << YAML::EndSeq << YAML::EndMap;
			toCreate << emitter.c_str();
			toCreate.close();
		}
	}

	void AssetManager::Reimport()
	{

		AssetManager::Reimport("assets");
	}

	void AssetManager::Reimport(const std::string& folder)
	{
		std::filesystem::path assetPath(folder);

		for (auto dirIt : std::filesystem::directory_iterator(folder))
		{
			if (dirIt.is_directory())
				Reimport(dirIt.path().string());
			else
			{
				// Open the meta file and load the ID
				std::ifstream meta(dirIt.path().string() + ".meta");
				std::stringstream ss;
				
				if (meta.good())
				{
					ss << meta.rdbuf();
					YAML::Node inYaml = YAML::Load(ss.str().c_str());

					uint64_t id = inYaml["ID"].as<uint64_t>();
					std::string path = dirIt.path().string();

					s_AssetMap[id] = path;
					AssetManager::AddAsset(id, path);
				}
			}
		}
	}

	void AssetManager::AddAsset(const UUID& id, const std::string& path)
	{
		std::stringstream ss;
		std::fstream currFile("Debut\\AssetMap.yaml", std::ios::in | std::ios::out);
		YAML::Emitter emitter;
		YAML::Emitter testEmitter;
		YAML::Node currYaml;
		YAML::Node newNode;

		// Read the file, load the old content
		ss << currFile.rdbuf();
		currYaml = YAML::Load(ss.str().c_str());

		// Create the new node and add it to the map
		newNode["ID"] = (uint64_t)id;
		newNode["Path"] = path;
		currYaml["Associations"].push_back(newNode);
		
		// Write the final document
		emitter << currYaml;
		emitter.SetSeqFormat(YAML::EMITTER_MANIP::Newline);

		currFile.close();
		currFile.open("Debut\\AssetMap.yaml", std::ios::out | std::ios::trunc);
		currFile << emitter.c_str();
	}

	// ASSET SUBMISSION BY PATH

	template <typename T>
	void AssetManager::Submit(const std::string& path)
	{

	}

	template <>
	void AssetManager::Submit<Texture2D>(const std::string& path)
	{
		if (s_TextureCache.Has(path))
			return;

		Ref<Texture2D> toAdd = Texture2D::Create(path);
		s_TextureCache.Put(path, toAdd);
	}

	template<>
	void AssetManager::Submit<PhysicsMaterial2D>(const std::string& path)
	{
		if (s_PhysicsMaterial2DCache.Has(path))
			return;

		Ref<PhysicsMaterial2D> toAdd = CreateRef<PhysicsMaterial2D>(path);
		s_PhysicsMaterial2DCache.Put(path, toAdd);
	}

	// ASSET SUBMISSION BY REFERENCE

	template <typename T>
	void AssetManager::Submit(const Ref<T>& toAdd)
	{

	}

	template <>
	void AssetManager::Submit<Texture2D>(const Ref<Texture2D>& texture)
	{
		if (s_TextureCache.Has(texture->GetPath()))
			return;

		s_TextureCache.Put(texture->GetPath(), texture);
		return;
	}

	template <>
	void AssetManager::Submit<PhysicsMaterial2D>(const Ref<PhysicsMaterial2D>& material)
	{
		if (s_PhysicsMaterial2DCache.Has(material->GetPath()))
			return;

		s_PhysicsMaterial2DCache.Put(material->GetPath(), material);
		return;
	}

	// ASSET REQUESTS

	template <typename T>
	Ref<T> AssetManager::Request(const std::string& id)
	{

	}

	template<>
	Ref<Texture2D> AssetManager::Request<Texture2D>(const std::string& id)
	{
		if (s_TextureCache.Has(id))
			return s_TextureCache.Get(id);

		Ref<Texture2D> toAdd;
		if (id == "")
			toAdd = Texture2D::Create(1, 1);
		else
			toAdd = Texture2D::Create(id);
		s_TextureCache.Put(id, toAdd);

		return toAdd;
	}

	template<>
	Ref<PhysicsMaterial2D> AssetManager::Request<PhysicsMaterial2D>(const std::string& id)
	{
		if (s_PhysicsMaterial2DCache.Has(id))
			return s_PhysicsMaterial2DCache.Get(id);

		Ref<PhysicsMaterial2D> toAdd = CreateRef<PhysicsMaterial2D>(id);
		s_PhysicsMaterial2DCache.Put(id, toAdd);

		return toAdd;
	}
}