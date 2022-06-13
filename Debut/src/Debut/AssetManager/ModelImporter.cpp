#include <Debut/AssetManager/ModelImporter.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Core/Log.h>
#include <Debut/Rendering/Resources/Mesh.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Debut/Rendering/Renderer/Renderer3D.h>


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
			Log.CoreError("Error while importing model {0}: {1}", path, importer.GetErrorString());
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
			aiMesh* assimpMesh = scene->mMeshes[parent->mMeshes[i]];
			aiMaterial* assimpMaterial = scene->mMaterials[assimpMesh->mMaterialIndex];

			Ref<Mesh> mesh = CreateRef<Mesh>(parent->mName);
			mesh->m_Vertices.resize(assimpMesh->mNumVertices);

			// Create mesh, start with the positions of the vertices
			for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
				mesh->m_Vertices[i] = {assimpMesh->mVertices[i].x, assimpMesh->mVertices[i].y, assimpMesh->mVertices[i].z};
			
			// Load normals
			if (assimpMesh->HasNormals())
			{
				mesh->m_Normals.resize(assimpMesh->mNormals->Length());
				for (uint32_t i = 0; i < assimpMesh->mNormals->Length(); i++)
					mesh->m_Normals[i] = { assimpMesh->mNormals[i].x, assimpMesh->mNormals[i].y, assimpMesh->mNormals[i].z };
			}

			// Load tangents / bitangents
			if (assimpMesh->HasTangentsAndBitangents())
			{
				mesh->m_Tangents.resize(assimpMesh->mTangents->Length());
				mesh->m_Bitangents.resize(assimpMesh->mBitangents->Length());

				for (uint32_t i = 0; i < assimpMesh->mNormals->Length(); i++)
				{
					mesh->m_Tangents[i] = { assimpMesh->mTangents[i].x, assimpMesh->mTangents[i].y, assimpMesh->mTangents[i].z };
					mesh->m_Bitangents[i] = { assimpMesh->mBitangents[i].x, assimpMesh->mBitangents[i].y, assimpMesh->mBitangents[i].z };
				}
			}

			// Load texture coordinates
			mesh->m_TexCoords.resize(assimpMesh->GetNumUVChannels());
			for (uint32_t i = 0; i < assimpMesh->GetNumUVChannels(); i++)
			{
				mesh->m_TexCoords[i].resize(assimpMesh->mTextureCoords[i]->Length());

				for (uint32_t j = 0; j < assimpMesh->mTextureCoords[i]->Length(); j++)
					mesh->m_TexCoords[i][j] = { assimpMesh->mTextureCoords[i][j].x, assimpMesh->mTextureCoords[i][j].y };
			}

			// Submit the mesh
			AssetManager::Submit<Mesh>(mesh);

			// Create the material
			aiMaterial* assimpMaterial = scene->mMaterials[assimpMesh->mMaterialIndex];
		}

		Ref<Model> ret = CreateRef<Model>(meshes, materials, models);
	}
}