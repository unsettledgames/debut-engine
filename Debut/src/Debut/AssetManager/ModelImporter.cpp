#include <Debut/AssetManager/ModelImporter.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Core/Log.h>
#include <Debut/Rendering/Resources/Mesh.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>


namespace Debut
{
	Ref<Model> ModelImporter::ImportModel(const std::string& path)
	{
		// TODO: have a look at importer settings
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(path, aiProcess_CalcTangentSpace | aiProcess_Triangulate |
												aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

		if (scene != nullptr)
		{
			aiNode* rootNode = scene->mRootNode;
			Ref<Model> ret = ImportNodes(rootNode, scene);
		}
		else
		{
			Log.CoreError("Error while importing model {9}: {1}", path, importer.GetErrorString());
			return nullptr;
		}
	}

	Ref<Model> ModelImporter::ImportNodes(aiNode* parent, const aiScene* scene)
	{
		std::vector<UUID> models;
		std::vector<UUID> meshes;
		std::vector<UUID> materials;

		meshes.resize(parent->mNumMeshes);
		materials.resize(parent->mNumMeshes);
		models.resize(parent->mNumChildren);

		// Load dependencies
		for (int i = 0; i < parent->mNumChildren; i++)
			models[i] = ModelImporter::ImportNodes(parent->mChildren[i], scene)->GetID();

		// Load meshes and materials
		for (int i = 0; i < parent->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[parent->mMeshes[i]];
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// Create material and mesh
		}

		Ref<Model> ret = CreateRef<Model>(meshes, materials, models);
	}
}