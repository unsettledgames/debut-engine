#include <Debut/AssetManager/ModelImporter.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Core/Log.h>
#include <cstdio>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Debut/Rendering/Renderer/Renderer3D.h>
#include <Debut/ImGui/ProgressPanel.h>
#include <Debut/Utils/CppUtils.h>

/*
	TODO
		UX:
			- Let the user reimport a model:
				- OPTIONAL: save a map <meshName, ID>; if during the reimporting, a mesh with the same name of the previous
					import is found, use the previous ID to save references
			- Save model import settings in .model.meta file
		- BUGS:
			- When reimporting with different settings, shader not found error
*/

namespace Debut
{
	Ref<Model> ModelImporter::ImportModel(const std::string& path, const ModelImportSettings& settings)
	{
		ProgressPanel::SubmitTask("modelimport", "Importing model...");
		std::string modelPath = path.substr(0, path.find_last_of("\\") + 1);
		modelPath += settings.ImportedName;

		std::ifstream meta(modelPath + ".model.meta");
		unsigned int pFlags = aiProcess_RemoveRedundantMaterials;

		if (meta.good())
		{
			meta.close();
			Ref<Model> model = AssetManager::Request<Model>(modelPath + ".model");
			std::vector<UUID> associationsToDelete;
			RemoveNodes(model, associationsToDelete);
			AssetManager::DeleteAssociations(associationsToDelete);
		}
		
		Assimp::Importer importer;

		if (settings.ImproveRenderingSpeed)
			pFlags |= aiProcess_ImproveCacheLocality;
		if (settings.JoinVertices)
			pFlags |= aiProcess_JoinIdenticalVertices;
		if (settings.Triangulate)
			pFlags |= aiProcess_Triangulate;
		if (settings.Normals)
			pFlags |= aiProcess_GenNormals;
		if (settings.TangentSpace)
			pFlags |= aiProcess_CalcTangentSpace;
		if (settings.OptimizeMeshes)
			pFlags |= aiProcess_OptimizeMeshes;
		if (settings.OptimizeScene)
			pFlags |= aiProcess_OptimizeGraph;

		const aiScene* scene = importer.ReadFile(path, pFlags);

		if (scene != nullptr)
		{
			// Import the model
			aiNode* rootNode = scene->mRootNode;
			Ref<Model> ret = ImportNodes(rootNode, scene, path.substr(0, path.find_last_of("\\")), settings.ImportedName);
			ret->SetPath(modelPath + ".model");

			ProgressPanel::CompleteTask("modelimport");
			meta.close();

			return ret;
		}
		else
		{
			Log.CoreError("Error while importing model {0}: {1}", path, importer.GetErrorString());
			return nullptr;
		}

		return nullptr;
	}

	Ref<Model> ModelImporter::ImportNodes(aiNode* parent, const aiScene* scene, const std::string& saveFolder, const std::string& modelName)
	{
		// Don't import empty models
		if (parent->mNumMeshes == 0 && parent->mNumChildren == 0)
			return nullptr;

		// Save the root model in the folder of the asset, save the generated assets in Lib
		std::string submodelsFolder = saveFolder;
		std::string assetsFolder = AssetManager::s_AssetsDir;
		if (parent->mParent != nullptr)
			submodelsFolder = AssetManager::s_AssetsDir;

		std::vector<UUID> models;
		std::vector<UUID> meshes;
		std::vector<UUID> materials;

		meshes.resize(parent->mNumMeshes);
		materials.resize(parent->mNumMeshes);
		models.resize(parent->mNumChildren);

		// Load dependencies
		for (int i = 0; i < parent->mNumChildren; i++)
		{
			ProgressPanel::ProgressTask("modelimport", i / parent->mNumChildren);
			Ref<Model> currModel = ImportNodes(parent->mChildren[i], scene, submodelsFolder);
			if (currModel != nullptr)
				models[i] = currModel->GetID();
			else
				models[i] = 0;
		}
		models.erase(std::remove(models.begin(), models.end(), 0), models.end());

		// Load meshes and materials
		for (int i = 0; i < parent->mNumMeshes; i++)
		{
			// Import and submit the mesh
			aiMesh* assimpMesh = scene->mMeshes[parent->mMeshes[i]];
			Ref<Mesh> mesh = ModelImporter::ImportMesh(assimpMesh, "Mesh" + i, assetsFolder);
			AssetManager::Submit<Mesh>(mesh);
			if (mesh != nullptr)
				meshes[i] = mesh->GetID();

			// Import and submit the material
			aiMaterial* assimpMaterial = scene->mMaterials[assimpMesh->mMaterialIndex];
			Ref<Material> material = ModelImporter::ImportMaterial(assimpMaterial, "Material" + i, assetsFolder);
			AssetManager::Submit<Material>(material);
			if (material != nullptr)
				materials[i] = material->GetID();
		}
		meshes.erase(std::remove(meshes.begin(), meshes.end(), 0), meshes.end());
		materials.erase(std::remove(materials.begin(), materials.end(), 0), materials.end());

		Ref<Model> ret = CreateRef<Model>(meshes, materials, models);
		std::stringstream ss;
		std::string name;
		if (parent->mParent == nullptr)
			name = modelName;
		else
			name = CppUtils::FileSystem::CorrectFileName(parent->mName.C_Str());
		ret->SetPath(submodelsFolder + "\\" + name + ".model");
		ret->SaveSettings();
		AssetManager::Submit<Model>(ret);

		return ret;
	}

	Ref<Mesh> ModelImporter::ImportMesh(aiMesh* assimpMesh, const std::string& name, const std::string& saveFolder)
	{
		ProgressPanel::SubmitTask("meshimport", "Importing mesh...");

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
		ProgressPanel::ProgressTask("meshimport", 0.17);

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
		ProgressPanel::ProgressTask("meshimport", 0.17);

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
			ProgressPanel::ProgressTask("meshimport", 0.17);

			for (uint32_t i = 0; i < assimpMesh->mNumVertices; i++)
			{
				for (uint32_t j = 0; j < 3; j++)
				{
					uint32_t index = i * 3 + j;
					mesh->m_Bitangents[index] = assimpMesh->mBitangents[i][j];
				}
			}
			ProgressPanel::ProgressTask("meshimport", 0.17);
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
		ProgressPanel::ProgressTask("meshimport", 0.17);

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
		ProgressPanel::ProgressTask("meshimport", 0.17);

		// Save the mesh on disk + meta file
		std::stringstream ss;
		ss << saveFolder << mesh->GetID();
		mesh->SetPath(ss.str());
		mesh->SetName(assimpMesh->mName.C_Str());
		mesh->SaveSettings();

		ProgressPanel::CompleteTask("meshimport");

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
		ss << saveFolder << material->GetID();
		material->SetPath(ss.str());
		ss.str("");

		ss << AssetManager::s_MetadataDir << material->GetID() << ".meta";
		material->SetMetaPath(ss.str());
		material->SaveSettings();

		return material;
	}

	void ModelImporter::RemoveNodes(Ref<Model> model, std::vector<UUID>& associations)
	{
		CppUtils::FileSystem::RemoveFile((model->GetPath() + ".meta").c_str());
		CppUtils::FileSystem::RemoveFile((model->GetPath()).c_str());
		AssetManager::Remove<Model>(model->GetID());
		associations.push_back(model->GetID());

		std::stringstream ss;
		for (uint32_t i = 0; i < model->GetMeshes().size(); i++)
		{
			ss.str("");
			ss << AssetManager::s_AssetsDir << model->GetMeshes()[i];
			CppUtils::FileSystem::RemoveFile(ss.str().c_str());
			associations.push_back(model->GetMeshes()[i]);
			AssetManager::Remove<Mesh>(model->GetMeshes()[i]);

			ss.str("");
			ss << AssetManager::s_MetadataDir << model->GetMeshes()[i] << ".meta";
			CppUtils::FileSystem::RemoveFile(ss.str().c_str());
		}

		for (uint32_t i = 0; i < model->GetMaterials().size(); i++)
		{
			ss.str("");
			ss << AssetManager::s_AssetsDir << model->GetMaterials()[i];
			CppUtils::FileSystem::RemoveFile(ss.str().c_str());
			associations.push_back(model->GetMaterials()[i]);
			AssetManager::Remove<Material>(model->GetMaterials()[i]);

			ss.str("");
			ss << AssetManager::s_MetadataDir << model->GetMaterials()[i] << ".meta";
			CppUtils::FileSystem::RemoveFile(ss.str().c_str());
		}

		for (uint32_t i = 0; i < model->GetSubmodels().size(); i++)
			RemoveNodes(AssetManager::Request<Model>(model->GetSubmodels()[i]), associations);
	}
}