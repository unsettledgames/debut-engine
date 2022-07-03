#pragma once

/* 
*	- Now, mesh files are a lot: I can just move them to a different folder (Lib/Metadata), even though it's only possible for
*	  certain assets, namely the ones that are generated by Debut. In fact, when Debut generates assets (Meshes, Materials), it's
*     responsible for them and knows that they won't be moved around.
* 
*	  -> The fact that only some assets can be put in the Lib/Assets folder is also hinted by the fact that Unity doesn't let you
*		 edit Materials belonging to a certain 3D model.
* 
*	  -> It's now important to split the Importing phase and the Loading phase. Checking if a model has already been imported should
*		 now happen when the user first attempts to load it: if there's a .meta file, the rest is loaded, otherwise it's imported.
*/

/*
	Let's go! Most of the stuff, at least regarding meshes is done.
	Now, before getting to shading and textures, I'd like to improve the current system a little bit. Here's a list of problems:
	
	1 - Loading models takes relatively a lot. How to address this?
		- YAML::Load is the current bottleneck. The only solution is to get rid of YAML and have my own data format.
	
	2 - The rendering process can probably be sped up a bit.
		- Avoid reallocating the index buffer every time new indices are submitted
		- Profile the PushData function, which is what takes the most

	3 - Find some way to have less files when importing a model
		- Unity keeps the same hierarchy in the scene view
		- Unity doesn't create files for materials or meshes????
			- Unity does create intermediate files. It saves them in the Library/Artifacts folder. It might be better to postpone
			  optimizing the amount of files created until I implement Projects.
			- This also explains why loading textures in Unity is easier! They get compressed so that when using them as icons, the 
			  loading times are lower.
			- In that way, I can also get rid of YAML and just use binary files
			- In addition, I don't need to have .meta files for Materials and Meshes: in fact, they just contain the ID of the resource,
			  which is unnecessary if I save the resource file using its UUID as the name
		- In my case I could just create a "Meshes", "Materials" and "Models" folders
		- In Unity, you can't import a part of a model, however you can import it, put it in the scene and then delete / hide submodels
		- A nice start could be to have all the model data into a single .model file

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