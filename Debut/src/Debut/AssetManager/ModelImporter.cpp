#include <Debut/AssetManager/ModelImporter.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Core/Log.h>

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
			AssetManager::SubmitModel(ret, path);
			return ret;
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
			// Import and submit the mesh
			aiMesh* assimpMesh = scene->mMeshes[parent->mMeshes[i]];
			AssetManager::Submit<Mesh>(ModelImporter::ImportMesh(assimpMesh, parent->mName.C_Str()));

			// Import and submit the material
			aiMaterial* assimpMaterial = scene->mMaterials[assimpMesh->mMaterialIndex];
			AssetManager::Submit<Material>(ModelImporter::ImportMaterial(assimpMaterial, assimpMaterial->GetName().C_Str()));
		}

		Ref<Model> ret = CreateRef<Model>(meshes, materials, models);
		AssetManager::Submit<Model>(ret);
		
		return ret;
	}

	Ref<Mesh> ModelImporter::ImportMesh(aiMesh* assimpMesh, const std::string& name)
	{
		Ref<Mesh> mesh = CreateRef<Mesh>(name);
		mesh->m_Vertices.resize(assimpMesh->mNumVertices);

		// Create mesh, start with the positions of the vertices
		for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
			mesh->m_Vertices[i] = { assimpMesh->mVertices[i].x, assimpMesh->mVertices[i].y, assimpMesh->mVertices[i].z };

		// Load normals
		if (assimpMesh->HasNormals())
		{
			mesh->m_Normals.resize(assimpMesh->mNumVertices);
			for (uint32_t i = 0; i < assimpMesh->mNormals->Length(); i++)
				mesh->m_Normals[i] = { assimpMesh->mNormals[i].x, assimpMesh->mNormals[i].y, assimpMesh->mNormals[i].z };
		}

		// Load tangents / bitangents
		if (assimpMesh->HasTangentsAndBitangents())
		{
			mesh->m_Tangents.resize(assimpMesh->mNumVertices);
			mesh->m_Bitangents.resize(assimpMesh->mNumVertices);

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
			mesh->m_TexCoords[i].resize(assimpMesh->mNumVertices);

			for (uint32_t j = 0; j < assimpMesh->mTextureCoords[i]->Length(); j++)
				mesh->m_TexCoords[i][j] = { assimpMesh->mTextureCoords[i][j].x, assimpMesh->mTextureCoords[i][j].y };
		}

		return mesh;
	}

	Ref<Material> ModelImporter::ImportMaterial(aiMaterial* assimpMaterial, const std::string& name)
	{
		// Configure the material
		Ref<Material> material = CreateRef<Material>();
		material->SetShader(AssetManager::Request<Shader>("assets\\shaders\\default-3d.glsl"));

		// Store / add properties
		aiMaterialProperty** properties = assimpMaterial->mProperties;

		// Colors
		aiColor3D color;
		assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->SetVec3("u_DiffuseColor", { color.r, color.g, color.b });

		assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
		material->SetVec3("u_AmbientColor", { color.r, color.g, color.b });

		// Textures
		if (assimpMaterial->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path;
			assimpMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path);
			material->SetTexture("u_DiffuseTexture", AssetManager::Request<Texture2D>(path.C_Str()));
		}

		if (assimpMaterial->GetTextureCount(aiTextureType_NORMALS))
		{
			aiString path;
			assimpMaterial->Get(AI_MATKEY_TEXTURE_NORMALS(0), path);
			material->SetTexture("u_NormalMap", AssetManager::Request<Texture2D>(path.C_Str()));
		}

		if (assimpMaterial->GetTextureCount(aiTextureType_DISPLACEMENT))
		{
			aiString path;
			assimpMaterial->Get(AI_MATKEY_TEXTURE_DISPLACEMENT(0), path);
			material->SetTexture("u_DisplacementMap", AssetManager::Request<Texture2D>(path.C_Str()));
		}

		return material;
	}
}