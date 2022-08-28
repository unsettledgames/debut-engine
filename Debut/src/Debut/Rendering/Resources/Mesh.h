#pragma once

#include <Debut/Core/UUID.h>

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

		inline UUID GetID() { return m_ID; }
		inline std::string GetName() { return m_Name; }
		inline std::string GetPath() { return m_Path; }
		inline bool IsValid() { return m_Valid; }

		inline std::vector<float>& GetPositions() { return m_Vertices; }
		inline std::vector<float>& GetNormals() { return m_Normals; }
		inline std::vector<float>& GetTangents() { return m_Tangents; }
		inline std::vector<float>& GetBitangents() { return m_Bitangents; }
		inline std::vector<float>& GetTexCoords(uint32_t index) { return m_TexCoords[index]; }
		inline std::vector<int>& GetIndices() { return m_Indices; }

		inline void SetPositions(std::vector<float>& vec) { m_Vertices = vec; }
		inline void SetNormals(std::vector<float>& vec) { m_Normals = vec; }
		inline void SetTangents(std::vector<float>& vec) { m_Tangents = vec; }
		inline void SetBitangents(std::vector<float>& vec) { m_Bitangents = vec; }
		inline void SetTexCoords(std::vector<float>& vec, uint32_t i) { m_TexCoords[i] = vec; }
		inline void SetIndices(std::vector<int>& vec) { m_Indices = vec; }

		inline bool HasNormals() { return m_Normals.size() > 0; }
		inline bool HasTangents() { return m_Tangents.size() > 0; }
		inline bool HasBitangents() { return m_Bitangents.size() > 0; }
		inline bool HasTexCoords(uint32_t index) { return m_TexCoords[index].size() > 0; }

		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetPath(const std::string& path) { m_Path = path; }
		inline void SetID(const UUID& id) { m_ID = id; }

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