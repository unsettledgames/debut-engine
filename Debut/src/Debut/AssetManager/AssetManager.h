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


/* THINK
*
*	- At the moment we're storing textures and physics materials by their path.
*		- In the future, assets will have to be packed and zipped. The engine will probably still refer to them using
*		  the normal path, as it makes quite sense to identify assets by saying "assets/textures/texture.png" as it also
*		  kinda describes the type of textures we want to load.
*
*			-> We could address this just by having the runtime version of the asset manager redirect requests from the virtual
*			   path to the physical path in the zipped file (Game Engine Architecture says it's possible).
*
*		- How do we handle renaming and moving assets? Let's start by assuming that the renaming / moving will happen from
*		  inside the engine, even though it could not happen sometimes.
*
*		  Since we're identifying assets by their path, of course, it'll be impossible to find them again once the path changes.
*		  BUT each asset either is a YAML file containing an ID or it's linked to a YAML file containing an ID. When something is moved
*		  from inside the engine, then the .meta file should be moved too. In this way, wherever an asset is, it still has its ID.
*
*			-> A naive solution maybe (it could be pretty expensive): when we boot the editor OR the game, a database of associations
*			   ID -> path is loaded. That database is kept up to date when something is moved or renamed, so that when an asset
*			   requests a resource with a certain ID, finding it is pretty straightforward
*
*			-> A solution I like more is having a folder that kinda replicates the structure of the user's Asset folder, let's call
*			   it DebutAssets. Let's assume that the user won't ever modify it. When loading an asset, the engine specifies the path
*			   relative to DebutAssets: the file (that is guaranteed to exist, since it's created the first time an asset is imported),
*			   contains the actual path to the desired file. The asset manager uses it and retrieves the right resource.
*
*		Dang I kinda like this format. Just realized that the first solution is probably better, since the only thing I serialize is the
*		ID of the resource and that's guaranteed to stay constant once an asset is created. When loading an asset, we have to load
*		its dependencies too: those are saved as IDs in it. Therefore we can just load the database and search in it. We can even
*		divide the database into multiple chunks to speed up the process and avoid loading the whole thing if it becomes too big.
*
*		Now, when do we use strings instead of IDs? Ideally only when we load stuff from a Script. And I think it's pretty acceptable
*		to return a huge error if the path changes (or at least that's what Unity does, so if it does it, I trust them that it's
*		acceptable). In fact, when we drag n drop stuff from the editor, we're working with paths, but those paths are immediately
*		linked to References and those don't randomly move lol, PLUS we can update the cache and use the right path instead.
*
*		The engine doesn't really use assets or paths, it's just the editor that needs path to configure stuff.
*
*		Soooo probably the most naive and cool approach, for now, is to have an asset database and hope that users don't mess it up
*		(why they should?) even though I don't like relying on the users not making mistakes.
*
*/

namespace Debut
{
	// TODO: maintain a file containing the UUID -> path associations for assets
	// After this one is done I could probably add some automatic asset import!
	class AssetManager
	{
	public:
		AssetManager() = default;

		static void Init();
		static void Reimport();

		static std::string GetPath(UUID id);
		static void AddAssociationToFile(const UUID& id, const std::string& path);

		template <typename T>
		static void Submit(Ref<T> asset) {}

		// TODO: make these 2 guys uise UUIDs instead of paths, make sure meta files are loaded correctly
		template <>
		static void Submit(Ref<Mesh> asset)
		{
			s_MeshCache.Put(asset->GetPath(), asset);
		}

		template <>
		static void Submit(Ref<Model> asset)
		{
			std::ostringstream os;
			os << asset->GetID();
			s_ModelCache.Put(os.str(), asset);
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
		static Ref<T> Request(const std::string& id);
		template <typename T>
		static Ref<T> Request(UUID id)
		{
			if (id == 0)
				return nullptr;
			if (s_AssetMap.find(id) == s_AssetMap.end())
				return nullptr;

			return Request<T>(s_AssetMap[id]);
		}

	private:
		static void Reimport(const std::string& folder);

		static std::unordered_map<UUID, std::string> s_AssetMap;
		static AssetCache<std::string, Ref<Texture2D>> s_TextureCache;
		static AssetCache<std::string, Ref<Shader>> s_ShaderCache;
		static AssetCache<std::string, Ref<Material>> s_MaterialCache;
		static AssetCache<std::string, Ref<Mesh>> s_MeshCache;
		static AssetCache<std::string, Ref<Model>> s_ModelCache;
	};
}
