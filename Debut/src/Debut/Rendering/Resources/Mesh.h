#pragma once

#include <Debut/Core/UUID.h>
#include <Debut/Rendering/Structures/VertexArray.h>

namespace Debut
{
	struct MeshVertex;

	class Mesh
	{
		friend class ModelImporter;
	public:
		Mesh();
		Mesh(const std::string& path);

		void SaveSettings();

		UUID GetID() { return m_ID; }
		std::string GetName() { return m_Name; }
		std::string GetName() { return m_Path; }

		std::vector<glm::vec3> GetPositions() { return m_Vertices; }
		std::vector<glm::vec3> GetNormals() { return m_Normals; }
		std::vector<glm::vec3> GetTangents() { return m_Tangents; }
		std::vector<glm::vec3> GetBitangents() { return m_Bitangents; }
		std::vector<glm::vec2> GetTexCoords(uint32_t index) { return m_TexCoords[index]; }
		std::vector<int> GetIndices() { return m_Indices; }

		void SetName(const std::string& name) { m_Name = name; }
		void SetPath(const std::string& path) { m_Path = path; }
		
	private:
		UUID m_ID;
		std::string m_Name;
		std::string m_Path;

		std::vector<glm::vec3> m_Vertices;
		std::vector<glm::vec3> m_Normals;
		std::vector<glm::vec3> m_Tangents;
		std::vector<glm::vec3> m_Bitangents;
		std::vector<std::vector<glm::vec2>> m_TexCoords;

		std::vector<int> m_Indices;
	};
}