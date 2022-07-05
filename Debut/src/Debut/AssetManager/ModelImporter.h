#pragma once

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