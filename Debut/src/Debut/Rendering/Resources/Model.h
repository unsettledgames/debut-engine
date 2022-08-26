#pragma once

#include <Debut/Core/UUID.h>

namespace Debut
{
	class Model
	{
	public:
		Model() {}
		Model(const std::string& model, const std::string& metaFile = "");
		Model(std::vector<UUID> meshes, std::vector<UUID> materials, std::vector<UUID> subModels) :
			m_Meshes(meshes), m_Materials(materials), m_SubModels(subModels) {}

		void SaveSettings();

		UUID GetID() { return m_ID; }
		std::string GetPath() { return m_Path; }
		std::vector<UUID> GetMeshes() { return m_Meshes; }
		std::vector<UUID> GetMaterials () { return m_Materials; }
		std::vector<UUID> GetSubmodels() { return m_SubModels; }
		bool IsValid() { return m_Valid; }

		void SetPath(const std::string& path) { m_Path = path; }

	private:
		UUID m_ID;
		bool m_Valid;
		std::string m_Path;

		std::vector<UUID> m_Materials;
		std::vector<UUID> m_Meshes;

		std::vector<UUID> m_SubModels;
	};
}