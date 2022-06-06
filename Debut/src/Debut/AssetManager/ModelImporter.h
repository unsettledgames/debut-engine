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
		1 - Load stuff with assimp
		2 - Get the necessary attributes
		3 - Pack the attributes in a vector of MeshVertices
		4 - Load the model
			4.1 - Submit a model to the renderer, copy the vector data so that it can be rendered
		5 - Save the new format!
			5.1 - Compress the vector
			5.2 - Create the .meta file
			5.3 - Save the metadata and the compressed vector
		6 - Load the model from the .meta file
			6.1 - Get the meta file and prepare the mesh
			6.2 - Decompress the vertex data
			6.3 - Store it in the mesh and prepare it for rendering
*/

#include <Debut/Core/Core.h>
#include <Debut/Rendering/Resources/Model.h>

#include <assimp/scene.h>

namespace Debut
{
	class ModelImporter
	{
	public:
		static Ref<Model> ImportModel(const std::string& path);
	private:
		static Ref<Model> ImportNodes(aiNode* parent, const aiScene* scene);
	};
}