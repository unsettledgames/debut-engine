#include <Debut/AssetManager/ModelImporter.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Core/Log.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Debut/Rendering/Renderer/Renderer3D.h>

/*
	TODO
		- Opzioni di importazione / dati nella sezione properties
*/

namespace Debut
{
	Ref<Model> ModelImporter::ImportModel(const std::string& path)
	{
		std::ifstream meta(path + ".model.meta");

		if (!meta.good())
		{
			Assimp::Importer importer;

			const aiScene* scene = importer.ReadFile(path, aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
			std::string folder = AssetManager::s_IntAssetsDir;
			std::string fileName;

			if (scene != nullptr)
			{
				// Import the model
				aiNode* rootNode = scene->mRootNode;
				Ref<Model> ret = ImportNodes(rootNode, scene, path.substr(0, path.find_last_of("\\")));

				return ret;
			}
			else
			{
				Log.CoreError("Error while importing model {0}: {1}", path, importer.GetErrorString());
				return nullptr;
			}
		}

		meta.close();
		return nullptr;
	}

	Ref<Model> ModelImporter::ImportNodes(aiNode* parent, const aiScene* scene, const std::string& saveFolder)
	{
		// Save the root model in the folder of the asset, save the generated assets in Lib
		std::string submodelsFolder = saveFolder;
		std::string assetsFolder = AssetManager::s_IntAssetsDir;
		if (parent->mParent != nullptr)
			submodelsFolder = AssetManager::s_IntAssetsDir;

		std::vector<UUID> models;
		std::vector<UUID> meshes;
		std::vector<UUID> materials;

		meshes.resize(parent->mNumMeshes);
		materials.resize(parent->mNumMeshes);
		models.resize(parent->mNumChildren);

		// Load dependencies
		for (int i = 0; i < parent->mNumChildren; i++)
		{
			Ref<Model> currModel = ImportNodes(parent->mChildren[i], scene, submodelsFolder);
			models[i] = currModel->GetID();
		}

		// Load meshes and materials
		for (int i = 0; i < parent->mNumMeshes; i++)
		{
			// Import and submit the mesh
			aiMesh* assimpMesh = scene->mMeshes[parent->mMeshes[i]];
			Ref<Mesh> mesh = ModelImporter::ImportMesh(assimpMesh, "Mesh" + i, assetsFolder);
			AssetManager::Submit<Mesh>(mesh);
			meshes[i] = mesh->GetID();

			// Import and submit the material
			aiMaterial* assimpMaterial = scene->mMaterials[assimpMesh->mMaterialIndex];
			Ref<Material> material = ModelImporter::ImportMaterial(assimpMaterial, "Material" + i, assetsFolder);
			AssetManager::Submit<Material>(material);
			materials[i] = material->GetID();
		}

		Ref<Model> ret = CreateRef<Model>(meshes, materials, models);
		ret->SetPath(submodelsFolder + "\\" + parent->mName.C_Str() + ".model");
		ret->SaveSettings();
		AssetManager::Submit<Model>(ret);
		
		return ret;
	}

	Ref<Mesh> ModelImporter::ImportMesh(aiMesh* assimpMesh, const std::string& name, const std::string& saveFolder)
	{
		Ref<Mesh> mesh = CreateRef<Mesh>();
		if (mesh->IsValid())
			return mesh;

		// Otherwise import it as usual
		mesh->m_Vertices.resize(assimpMesh->mNumVertices * 3);
		mesh->SetName(assimpMesh->mName.C_Str());

		// Create mesh, start with the positions of the vertices
		for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
			for (uint32_t j = 0; j < 3; j++)
			{
				uint32_t index = i * 3 + j;
				mesh->m_Vertices[index] = assimpMesh->mVertices[i][j];
			}

		// Load normals
		if (assimpMesh->HasNormals())
		{
			mesh->m_Normals.resize(assimpMesh->mNumVertices * 3);
			for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
				for (uint32_t j = 0; j < 3; j++)
				{
					uint32_t index = i * 3 + j;
					mesh->m_Normals[index] = assimpMesh->mNormals[i][j];
				}
		}

		// Load tangents / bitangents
		if (assimpMesh->HasTangentsAndBitangents())
		{
			mesh->m_Tangents.resize(assimpMesh->mNumVertices * 3);
			mesh->m_Bitangents.resize(assimpMesh->mNumVertices * 3);

			for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
			{
				for (uint32_t j = 0; j < 3; j++)
				{
					uint32_t index = i * 3 + j;
					mesh->m_Tangents[index] = assimpMesh->mTangents[i][j];
				}
			}

			for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
			{
				for (uint32_t j = 0; j < 3; j++)
				{
					uint32_t index = i * 3 + j;
					mesh->m_Bitangents[index] = assimpMesh->mBitangents[i][j];
				}
			}
		}

		// Load texture coordinates
		mesh->m_TexCoords.resize(assimpMesh->GetNumUVChannels());
		for (uint32_t i = 0; i < assimpMesh->GetNumUVChannels(); i++)
		{
			mesh->m_TexCoords[i].resize(assimpMesh->mNumVertices * 3);

			for (uint32_t j = 0; j < assimpMesh->mNumVertices; j++)
				for (uint32_t k = 0; k < 3; k++)
				{
					uint32_t index = j * 3 + k;
					mesh->m_TexCoords[i][index] = assimpMesh->mTextureCoords[i][j][k];
				}
		}

		// Load indices
		mesh->m_Indices.resize(assimpMesh->mNumFaces * 3);
		uint32_t indexIndex = 0;
		for (uint32_t i = 0; i < assimpMesh->mNumFaces; i++)
		{
			for (uint32_t j = 0; j < assimpMesh->mFaces[i].mNumIndices; j++)
			{
				mesh->m_Indices[indexIndex] = assimpMesh->mFaces[i].mIndices[j];
				indexIndex++;
			}
		}

		// Save the mesh on disk + meta file
		std::stringstream ss;
		ss << saveFolder << "\\" << mesh->GetID();
		mesh->SetPath(ss.str());
		mesh->SaveSettings();

		return mesh;
	}

	Ref<Material> ModelImporter::ImportMaterial(aiMaterial* assimpMaterial, const std::string& name, const std::string& saveFolder)
	{
		Ref<Material> material = CreateRef<Material>();
		if (material->IsValid())
			return material;

		// Otherwise import the material as usual
		material->SetShader(AssetManager::Request<Shader>("assets\\shaders\\default-3d.glsl", "assets\\shaders\\default-3d.glsl.meta"));
		material->SetName(assimpMaterial->GetName().C_Str());

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
			material->SetTexture("u_DiffuseTexture", AssetManager::Request<Texture2D>(path.C_Str(), std::string(path.C_Str()) + ".meta"));
		}

		if (assimpMaterial->GetTextureCount(aiTextureType_NORMALS))
		{
			aiString path;
			assimpMaterial->Get(AI_MATKEY_TEXTURE_NORMALS(0), path);
			material->SetTexture("u_NormalMap", AssetManager::Request<Texture2D>(path.C_Str(), std::string(path.C_Str()) + ".meta"));
		}

		if (assimpMaterial->GetTextureCount(aiTextureType_DISPLACEMENT))
		{
			aiString path;
			assimpMaterial->Get(AI_MATKEY_TEXTURE_DISPLACEMENT(0), path);
			material->SetTexture("u_DisplacementMap", AssetManager::Request<Texture2D>(path.C_Str(), std::string(path.C_Str()) + ".meta"));
		}

		// Save the material on disk + meta file
		// Save the mesh on disk + meta file
		std::stringstream ss;
		ss << saveFolder << "\\" << material->GetID();
		material->SetPath(ss.str());
		ss.str("");
		ss << AssetManager::s_ProjectDir << "\\Lib\\Metadata\\" << material->GetID() << ".meta";
		material->SetMetaPath(ss.str());
		material->SaveSettings();

		return material;
	}
}