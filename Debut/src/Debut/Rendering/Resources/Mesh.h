#pragma once

#include <Debut/Core/UUID.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <yaml-cpp/yaml.h>

namespace Debut
{
	struct MeshMetadata
	{
		std::string Name;
		UUID ID;
	};

	class Mesh
	{
		friend class ModelImporter;
	public:
		Mesh();
		Mesh(const std::string& path, const std::string& metaPath);

		void SaveSettings();

		UUID GetID() { return m_ID; }
		std::string GetName() { return m_Name; }
		std::string GetPath() { return m_Path; }
		bool IsValid() { return m_Valid; }

		std::vector<float>& GetPositions() { return m_Vertices; }
		std::vector<float>& GetNormals() { return m_Normals; }
		std::vector<float>& GetTangents() { return m_Tangents; }
		std::vector<float>& GetBitangents() { return m_Bitangents; }
		std::vector<float>& GetTexCoords(uint32_t index) { return m_TexCoords[index]; }
		std::vector<int>& GetIndices() { return m_Indices; }

		void SetPositions(std::vector<float>& vec) { m_Vertices = vec; }
		void SetNormals(std::vector<float>& vec) { m_Normals = vec; }
		void SetTangents(std::vector<float>& vec) { m_Tangents = vec; }
		void SetBitangents(std::vector<float>& vec) { m_Bitangents = vec; }
		void SetTexCoords(std::vector<float>& vec, uint32_t i) { m_TexCoords[i] = vec; }
		void SetIndices(std::vector<int>& vec) { m_Indices = vec; }

		bool HasNormals() { return m_Normals.size() > 0; }
		bool HasTangents() { return m_Tangents.size() > 0; }
		bool HasBitangents() { return m_Bitangents.size() > 0; }
		bool HasTexCoords(uint32_t index) { return m_TexCoords[index].size() > 0; }

		void SetName(const std::string& name) { m_Name = name; }
		void SetPath(const std::string& path) { m_Path = path; }
		void SetID(const UUID& id) { m_ID = id; }

		static MeshMetadata GetMetadata(UUID id);

	private:
		void Load(std::ifstream& inFile);
		
	private:
		UUID m_ID;
		bool m_Valid;

		std::string m_Name;
		std::string m_Path;
		std::string m_MetaPath;

		std::vector<float> m_Vertices;
		std::vector<float> m_Normals;
		std::vector<float> m_Tangents;
		std::vector<float> m_Bitangents;
		std::vector<std::vector<float>> m_TexCoords;

		std::vector<int> m_Indices;
	};
}