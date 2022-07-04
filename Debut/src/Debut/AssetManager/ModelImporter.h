#pragma once

/* 
*	  -> It's now important to split the Importing phase and the Loading phase. Checking if a model has already been imported should
*		 now happen when the user first attempts to load it: if there's a .meta file, the rest is loaded, otherwise it's imported.
*/

/*
	Let's go! Most of the stuff, at least regarding meshes is done.
	Now, before getting to shading and textures, I'd like to improve the current system a little bit. Here's a list of problems:
	
	2 - The rendering process can probably be sped up a bit.
		- Avoid reallocating the index buffer every time new indices are submitted
		- Profile the PushData function, which is what takes the most

	4 - Find out why sometimes textures don't load
*/

#include <Debut/Core/Core.h>

#include <Debut/Rendering/Material.h>
#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/Rendering/Resources/Model.h>

#include <assimp/scene.h>

namespace Debut
{
	class ModelImporter
	{
	public:
		static Ref<Model> ImportModel(const std::string& path);
	private:
		static Ref<Model> ImportNodes(aiNode* parent, const aiScene* scene, const std::string& saveFolder);
		static Ref<Mesh> ImportMesh(aiMesh* mesh, const std::string& name, const std::string& saveFolder);
		static Ref<Material> ImportMaterial(aiMaterial* material, const std::string& name, const std::string& saveFolder);
	};
}