#include <Debut/AssetManager/ModelImporter.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Core/Log.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Debut/Rendering/Renderer/Renderer3D.h>

/*
	TODO
		- Integrare ModelImporter con AssetManager

		- Salvare le informazioni di materiali e mesh su disco con file .meta, da questo punto in poi l'integrazione è quasi completa
			probabilmente
		- Infine, salvare le informazioni del modello
		- Al caricamento di un modello, verificare che non sia già stato importato
		- Modificare Request in AssetManager per i template del caso
		- Opzioni di importazione / dati nella sezione properties

	ALTRE NOTE:
		- Un modello non è altro che un singolo file che descrive la gerarchia del modello usando gli UUID
*/

namespace Debut
{
	Ref<Model> ModelImporter::ImportModel(const std::string& path)
	{
		// TODO: have a look at importer settings
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(path, aiProcess_CalcTangentSpace | aiProcess_Triangulate |
												aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
		std::string folder = path.substr(0, path.find_last_of('\\'));
		std::string fileName;

		if (scene != nullptr)
		{
			// Import the model
			aiNode* rootNode = scene->mRootNode;
			Ref<Model> ret = ImportNodes(rootNode, scene, folder);
			AssetManager::SubmitModel(ret, path);

			return ret;
		}
		else
		{
			Log.CoreError("Error while importing model {0}: {1}", path, importer.GetErrorString());
			return nullptr;
		}
	}

	Ref<Model> ModelImporter::ImportNodes(aiNode* parent, const aiScene* scene, const std::string& saveFolder)
	{
		std::vector<UUID> models;
		std::vector<UUID> meshes;
		std::vector<UUID> materials;

		meshes.resize(parent->mNumMeshes);
		materials.resize(parent->mNumMeshes);
		models.resize(parent->mNumChildren);

		// Load dependencies
		for (int i = 0; i < parent->mNumChildren; i++)
			models[i] = ModelImporter::ImportNodes(parent->mChildren[i], scene, saveFolder)->GetID();

		// Load meshes and materials
		for (int i = 0; i < parent->mNumMeshes; i++)
		{
			// Import and submit the mesh
			aiMesh* assimpMesh = scene->mMeshes[parent->mMeshes[i]];
			AssetManager::Submit<Mesh>(ModelImporter::ImportMesh(assimpMesh, "Mesh" + i, saveFolder));

			// Import and submit the material
			aiMaterial* assimpMaterial = scene->mMaterials[assimpMesh->mMaterialIndex];
			AssetManager::Submit<Material>(ModelImporter::ImportMaterial(assimpMaterial, "Material" + i, saveFolder));
		}

		Ref<Model> ret = CreateRef<Model>(meshes, materials, models);
		AssetManager::Submit<Model>(ret);
		
		return ret;
	}

	Ref<Mesh> ModelImporter::ImportMesh(aiMesh* assimpMesh, const std::string& name, const std::string& saveFolder)
	{
		// Check if the mesh has already been imported: if so, just return the already present version
		std::string meshPath;
		if (assimpMesh->mName.C_Str() == "")
			meshPath = saveFolder + "\\" + name;
		else
			meshPath = saveFolder + "\\" + assimpMesh->mName.C_Str();

		Ref<Mesh> mesh = AssetManager::Request<Mesh>(meshPath);
		if (mesh != nullptr)
			return mesh;

		// Otherwise import it as usual
		mesh = CreateRef<Mesh>(name);
		mesh->m_Vertices.resize(assimpMesh->mNumVertices);
		mesh->SetName(assimpMesh->mName.C_Str());

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

		// Save the mesh on disk + meta file
		if (mesh->GetName() == "")
			mesh->SetName(name);
		mesh->SetPath(saveFolder + "\\" + mesh->GetName());
		mesh->SaveSettings();

		return mesh;
	}

	Ref<Material> ModelImporter::ImportMaterial(aiMaterial* assimpMaterial, const std::string& name, const std::string& saveFolder)
	{
		// Check if the material has already been imported: if so, just return the already present version
		std::string materialPath;
		if (assimpMaterial->GetName().C_Str() == "")
			materialPath = saveFolder + "\\" + name;
		else
			materialPath = saveFolder + "\\" + assimpMaterial->GetName().C_Str();

		Ref<Material> material = AssetManager::Request<Material>(materialPath);
		if (material != nullptr)
			return material;

		// Otherwise import the material as usual
		material = CreateRef<Material>();
		material->SetShader(AssetManager::Request<Shader>("assets\\shaders\\default-3d.glsl"));
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

		// Save the material on disk + meta file
		if (material->GetName() == "")
			material->SetName(name);
		material->SetPath(saveFolder + "\\" + material->GetName());
		material->SaveSettings();
		// Add the association in the asset manager
		AssetManager::AddAssociationToFile(material->GetID(), material->GetPath());

		return material;
	}
}