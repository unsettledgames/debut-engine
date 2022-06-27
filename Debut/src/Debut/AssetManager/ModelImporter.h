#pragma once

/*
	So, how should this thing work?
	-	Should I save metadata for a whole model instead of a file per mesh? I should probably have a file per mesh considering that
		submeshes are a thing. I'm a bit afraid of polluting the file system with all of this metadata, but in the end the content
		browser can just hide them like I'm doing at the moment. In addition, having access to additional metadata I could even
		make the content browser a bit more stylish (hierarchies of meshes). I could even enable submeshing for formats that don't
		support it if I wanted to.

			-> Save metadata for each mesh
			-> Save metadata for a whole model
	-	

	So, roadmap:
DONE		1 - Load stuff with assimp DONE
DONE		2 - Get the necessary attributes DONE
DONE		3 - Pack the attributes in a vector of MeshVertices DONE
DONE		4 - Load the model
DONE			4.1 - Submit a model to the renderer, copy the vector data so that it can be rendered DONE
DONE		5 - Save the new format!
TODO			5.1 - Compress the vector
DONE			5.2 - Create the .meta file	DONE
DONISH			5.3 - Save the metadata and the compressed vector
DONE		6 - Load the model from the .meta file
DONE			6.1 - Get the meta file and prepare the mesh
TODO			6.2 - Decompress the vertex data 
DONE			6.3 - Store it in the mesh and prepare it for rendering

	Let's go! Most of the stuff, at least regarding meshes is done.
	Now, before getting to shading and textures, I'd like to improve the current system a little bit. Here's a list of problems:
	
	1 - Loading models takes relatively a lot. How to address this?
		- YAML::Load is the current bottleneck. The first solution would be to compress the data before saving it, then decompressing.
		  I could probably decompress the data straight into the buffer to set to spare a memcpy.
	
	2 - The rendering process can probably be sped up a bit.
		- Avoid reallocating the index buffer every time new indices are submitted
		- Profile the PushData function, which is what takes the most

	3 - Find some way to have less files when importing a model
		- Unity keeps the same hierarchy in the scene view
		- Unity doesn't create files for materials or meshes????
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