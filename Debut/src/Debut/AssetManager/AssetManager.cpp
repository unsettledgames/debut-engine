#include <Debut/dbtpch.h>
#include <yaml-cpp/yaml.h>
#include <imgui.h>
#include <filesystem>
#include <Debut/AssetManager/ModelImporter.h>
#include <Debut/AssetManager/AssetManager.h>

namespace Debut
{
	std::unordered_map<UUID, std::string> AssetManager::s_AssetMap;
	std::string AssetManager::s_ProjectDir;
	std::string AssetManager::s_AssetsDir;
	std::string AssetManager::s_MetadataDir;

	AssetCache<std::string, Ref<Texture2D>> AssetManager::s_TextureCache;
	AssetCache<std::string, Ref<Shader>> AssetManager::s_ShaderCache;
	AssetCache<std::string, Ref<Material>> AssetManager::s_MaterialCache;
	AssetCache<std::string, Ref<Mesh>> AssetManager::s_MeshCache;
	AssetCache<std::string, Ref<Model>> AssetManager::s_ModelCache;

	static AssetCache<std::string, Ref<PhysicsMaterial2D>> s_PhysicsMaterial2DCache;

	void AssetManager::Init(const std::string& projectDir)
	{
		s_ProjectDir = projectDir;
		s_AssetsDir = s_ProjectDir + "\\Lib\\Assets\\";
		s_MetadataDir = s_ProjectDir + "\\Lib\\Metadata\\";

		CreateLibDirs();

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

			Reimport("assets");
		}
	}

	void AssetManager::CreateLibDirs()
	{
		std::filesystem::path workingDir(s_ProjectDir + "\\Lib");

		if (!std::filesystem::exists(workingDir))
		{
			std::filesystem::create_directory(workingDir);
			std::filesystem::path assetDir(s_ProjectDir + "\\Lib\\Assets");
			std::filesystem::create_directory(assetDir);

			std::filesystem::path metaDir(s_ProjectDir + "\\Lib\\Metadata");
			std::filesystem::create_directory(metaDir);
		}
	}

	void AssetManager::Reimport()
	{
		std::ofstream file("Debut\\AssetMap.yaml", std::ios::out | std::ios::trunc);
		file.close();

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
					AssetManager::AddAssociationToFile(id, path);
				}
			}
		}
	}

	void AssetManager::AddAssociationToFile(const UUID& id, const std::string& path)
	{
		// Don't bother saving runtime assets
		if (path == "")
			return;

		std::stringstream ss;
		std::fstream currFile("Debut\\AssetMap.yaml", std::ios::in | std::ios::out);
		YAML::Emitter emitter;
		YAML::Emitter testEmitter;
		YAML::Node currYaml;
		YAML::Node newNode;

		s_AssetMap[id] = path;

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

	std::string AssetManager::GetPath(UUID id)
	{
		if (s_AssetMap.find(id) != s_AssetMap.end())
			return s_AssetMap[id];	
		return "None";
	}

	// ASSET SUBMISSION BY PATH

	// ASSET REQUESTS

	template <typename T>
	Ref<T> AssetManager::Request(const std::string& id, const std::string& metaFile)
	{

	}

	template<>
	Ref<Texture2D> AssetManager::Request<Texture2D>(const std::string& id, const std::string& metaFile)
	{
		if (s_TextureCache.Has(id))
			return s_TextureCache.Get(id);

		Ref<Texture2D> toAdd;
		if (id == "")
			toAdd = Texture2D::Create(1, 1);
		else
			toAdd = Texture2D::Create(id, metaFile);

		s_TextureCache.Put(id, toAdd);
		if (s_AssetMap.find(toAdd->GetID()) == s_AssetMap.end())
		{
			s_AssetMap[toAdd->GetID()] = id;
			AssetManager::AddAssociationToFile(toAdd->GetID(), id);
		}

		return toAdd;
	}

	template<>
	Ref<PhysicsMaterial2D> AssetManager::Request<PhysicsMaterial2D>(const std::string& id, const std::string& metaFile)
	{
		if (s_PhysicsMaterial2DCache.Has(id))
			return s_PhysicsMaterial2DCache.Get(id);

		Ref<PhysicsMaterial2D> toAdd = CreateRef<PhysicsMaterial2D>(id, metaFile);

		// Update the asset map if the entry wasn't there
		s_PhysicsMaterial2DCache.Put(id, toAdd);
		if (s_AssetMap.find(toAdd->GetID()) == s_AssetMap.end())
		{
			s_AssetMap[toAdd->GetID()] = id;
			AssetManager::AddAssociationToFile(toAdd->GetID(), id);
		}

		return toAdd;
	}

	template <>
	Ref<Shader> AssetManager::Request<Shader>(const std::string& id, const std::string& metaFile)
	{
		if (s_ShaderCache.Has(id))
			return s_ShaderCache.Get(id);

		Ref<Shader> toAdd = Shader::Create(id, metaFile);

		// Update the asset map if the entry wasn't there
		s_ShaderCache.Put(id, toAdd);
		if (s_AssetMap.find(toAdd->GetID()) == s_AssetMap.end())
		{
			s_AssetMap[toAdd->GetID()] = id;
			AssetManager::AddAssociationToFile(toAdd->GetID(), id);
		}

		return toAdd;
	}

	template <>
	Ref<Material> AssetManager::Request<Material>(const std::string& id, const std::string& metaFile)
	{
		if (s_MaterialCache.Has(id))
			return s_MaterialCache.Get(id);

		Ref<Material> toAdd = CreateRef<Material>(id, metaFile);

		// Update the asset map if the entry wasn't there
		s_MaterialCache.Put(id, toAdd);
		if (s_AssetMap.find(toAdd->GetID()) == s_AssetMap.end())
		{
			s_AssetMap[toAdd->GetID()] = id;
			AssetManager::AddAssociationToFile(toAdd->GetID(), id);
		}

		return toAdd;
	}

	template <>
	Ref<Mesh> AssetManager::Request<Mesh>(const std::string& id, const std::string& metaFile)
	{
		if (s_MeshCache.Has(id))
			return s_MeshCache.Get(id);

		Ref<Mesh> toAdd = CreateRef<Mesh>(id, metaFile);

		s_MeshCache.Put(id, toAdd);
		if (s_AssetMap.find(toAdd->GetID()) == s_AssetMap.end())
		{
			s_AssetMap[toAdd->GetID()] = id;
			AssetManager::AddAssociationToFile(toAdd->GetID(), id);
		}

		return toAdd;
	}
	
	template <>
	Ref<Model> AssetManager::Request<Model>(const std::string& id, const std::string& metaFile)
	{
		if (s_ModelCache.Has(id))
			return s_ModelCache.Get(id);

		Ref<Model> toAdd = CreateRef<Model>(id, metaFile);
		if (toAdd->IsValid())
		{
			// Load submodels too
			for (uint32_t i = 0; i < toAdd->GetSubmodels().size(); i++)
				AssetManager::Request<Model>(toAdd->GetSubmodels()[i]);
		}

		// Update the asset map if the entry wasn't there
		s_ModelCache.Put(id, toAdd);
		if (s_AssetMap.find(toAdd->GetID()) == s_AssetMap.end())
		{
			s_AssetMap[toAdd->GetID()] = id;
			AssetManager::AddAssociationToFile(toAdd->GetID(), id);
		}

		return toAdd;
	}

	std::vector<std::pair<UUID, std::string>> AssetManager::GetAssetMap()
	{
		std::vector<std::pair<UUID, std::string>> ret;

		for (auto& entry : s_AssetMap)
			ret.push_back(std::make_pair(entry.first, entry.second));

		std::sort(ret.begin(), ret.end(), 
			[](const std::pair<UUID, std::string>& lhs, const std::pair<UUID, std::string>& rhs) {
				return lhs.second.compare(rhs.second) < 0; 
			});

		return ret;
	}

	void AssetManager::DeleteAssociations(std::vector<UUID>& id)
	{
#ifdef DBT_DEBUG
		std::stringstream ss;
		std::fstream currFile("Debut\\AssetMap.yaml", std::ios::in | std::ios::out);
		YAML::Emitter emitter;
		YAML::Emitter testEmitter;
		YAML::Node currYaml;
		YAML::Node newNode;

		// Read the file, load the old content
		ss << currFile.rdbuf();
		currYaml = YAML::Load(ss.str().c_str())["Associations"];
		for (auto& node : currYaml)
		{
			if (std::find(id.begin(), id.end(), node["ID"].as<uint64_t>()) != id.end())
			{
				s_AssetMap.erase(node["ID"].as<uint64_t>());
				currYaml.remove(node.as<YAML::Node>());
			}
		}

		// Write the final document

		emitter << YAML::BeginMap << YAML::Key << "Associations" << YAML::Value << currYaml << YAML::EndMap;

		currFile.close();
		currFile.open("Debut\\AssetMap.yaml", std::ios::out | std::ios::trunc);
		currFile << emitter.c_str();

#endif
	}
	
}