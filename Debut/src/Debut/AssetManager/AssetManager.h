#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/AssetManager/Asset.h>
#include <Debut/AssetManager/AssetCache.h>
#include <Debut/Physics/PhysicsMaterial2D.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Resources/Model.h>

namespace Debut
{
	class AssetManager
	{
	public:
		AssetManager() = default;

		static void Init(const std::string& projectDir);
		static void Reimport();

		static std::string GetPath(UUID id);
		static void AddAssociationToFile(const UUID& id, const std::string& path);

		template <typename T>
		static void Submit(Ref<T> asset) {}
		
		template <typename T>
		static void Remove(UUID id) {}

		// TODO: make these 2 guys uise UUIDs instead of paths, make sure meta files are loaded correctly
		template <>
		static void Submit(Ref<Mesh> asset) { s_MeshCache.Put(asset->GetPath(), asset); }
		template <>
		static void Submit(Ref<Material> asset) { s_MaterialCache.Put(asset->GetPath(), asset); }
		template <>
		static void Submit(Ref<Model> model)
		{
			s_ModelCache.Put(model->GetPath(), model);
			AddAssociationToFile(model->GetID(), model->GetPath());
		}

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
		}


		template <typename T>
		static Ref<T> Request(const std::string& file, const std::string& metaFile = "");
		template <typename T>
		static Ref<T> Request(UUID id)
		{
			if (id == 0)
				return nullptr;

			std::stringstream ss;
			ss << s_AssetsDir << id;
			std::stringstream metaSs;
			metaSs << s_MetadataDir << id << ".meta";

			std::ifstream file(ss.str());
			if (file.good())
				return Request<T>(ss.str(), metaSs.str());

			if (s_AssetMap.find(id) == s_AssetMap.end())
				return nullptr;

			return Request<T>(s_AssetMap[id], s_AssetMap[id] + ".meta");
		}

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
		
	};
}
