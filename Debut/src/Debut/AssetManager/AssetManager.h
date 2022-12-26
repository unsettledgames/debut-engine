#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>
#include <Debut/AssetManager/AssetCache.h>
#include <Debut/Rendering/Resources/Model.h>

#define DBT_WHITE_TEXTURE_UUID	1

// Untemplate some templates so you can forward declare stuff

namespace Debut
{
	class Texture;
	class Texture2D;
	class PhysicsMaterial2D;
	class PhysicsMaterial3D;
	class Shader;
	class Material;
	class Mesh;
	class Model;
	class Skybox;
	class PostProcessingStack;

	class AssetManager
	{
	public:
		AssetManager() = default;

		static void Init(const std::string& projectDir);
		static void Reimport();

		static std::string GetPath(UUID id);
		static void AddAssociationToFile(const UUID& id, const std::string& path);

		static void Submit(Ref<Mesh> asset);
		static void Submit(Ref<Material> asset);
		static void Submit(Ref<Model> model);

		template <typename T>
		static void Remove(UUID id) {}

		template <>
		static void Remove<Mesh>(UUID id) 
		{ 
			if (s_MeshCache.Has(GetPath(id)))
				s_MeshCache.Remove(GetPath(id)); 
		}
		template <>
		static void Remove<Model>(UUID id) 
		{ 
			if (s_ModelCache.Has(GetPath(id)))
				s_ModelCache.Remove(GetPath(id)); 
		}
		template <>
		static void Remove<Material>(UUID id) 
		{ 
			if (s_MaterialCache.Has(GetPath(id)))
				s_MaterialCache.Remove(GetPath(id)); 
		}

		template<typename T>
		static void CreateAsset(const std::string& path)
		{
			std::string tmpPath = path;
			std::ifstream test(tmpPath);
			while (test.good())
			{
				// Append "-copy" before the extension to avoid overwriting stuff
				tmpPath = tmpPath.substr(0, tmpPath.find_last_of(".")) + "-copy" + tmpPath.substr(tmpPath.find_last_of("."), tmpPath.length() - tmpPath.find_last_of("."));
				test = std::ifstream(tmpPath);
			}
			
			// Create the empty file
			T::SaveDefaultConfig(tmpPath);
			// Load the ID from the newly created meta and add the association
			std::ifstream meta(tmpPath + ".meta");
			std::stringstream ss; ss << meta.rdbuf();
			YAML::Node node = YAML::Load(ss.str());
			AddAssociationToFile(node["ID"].as<uint64_t>(), path);
		}


		template <typename T>
		static Ref<T> Request(const std::string& file, const std::string& metaFile = "");
		template <typename T>
		static Ref<T> Request(UUID id);

		static std::vector<std::pair<UUID, std::string>> GetAssetMap();
		static void DeleteAssociations(std::vector<UUID>& id);

	public:
		// Working directory for the currently opened project
		static std::string s_ProjectDir;
		// Asset directory
		static std::string s_AssetsDir;
		// Metadata directory
		static std::string s_MetadataDir;
		
	private:
		static void Reimport(const std::string& folder);
		static void CreateLibDirs();

	private:
		static std::unordered_map<UUID, std::string> s_AssetMap;
		static AssetCache<std::string, Ref<Texture2D>> s_TextureCache;
		static AssetCache<std::string, Ref<Shader>> s_ShaderCache;
		static AssetCache<std::string, Ref<Material>> s_MaterialCache;
		static AssetCache<std::string, Ref<Mesh>> s_MeshCache;
		static AssetCache<std::string, Ref<Model>> s_ModelCache;
		static AssetCache<std::string, Ref<Skybox>> s_SkyboxCache;
		static AssetCache<std::string, Ref<PhysicsMaterial2D>> s_PhysicsMaterial2DCache;
		static AssetCache<std::string, Ref<PhysicsMaterial3D>> s_PhysicsMaterial3DCache;
		static AssetCache<std::string, Ref<PostProcessingStack>> s_PostProcessingStackCache;
		
	};
}
