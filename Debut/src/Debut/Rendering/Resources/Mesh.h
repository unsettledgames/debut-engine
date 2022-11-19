#pragma once

#include <Debut/Core/UUID.h>
#include <glm/glm.hpp>

#include <unordered_map>

namespace Debut
{
	class VertexArray;
	class VertexBuffer;
	class IndexBuffer;

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
		~Mesh();

		void SaveSettings();

		inline UUID GetID() { return m_ID; }
		inline std::string GetName() { return m_Name; }
		inline std::string GetPath() { return m_Path; }
		inline bool IsValid() { return m_Valid; }

		inline std::vector<float>& GetPositions() { return m_Vertices; }
		inline std::vector<float>& GetColors() { return m_Colors; }
		inline std::vector<float>& GetNormals() { return m_Normals; }
		inline std::vector<float>& GetTangents() { return m_Tangents; }
		inline std::vector<float>& GetBitangents() { return m_Bitangents; }
		inline std::vector<float>& GetTexCoords(uint32_t index) { return m_TexCoords[index]; }
		inline std::vector<int>& GetIndices() { return m_Indices; }
		inline Ref<VertexArray> GetVertexArray() { return m_VertexArray; }
		inline glm::mat4& GetTransform() { return m_Transform; }
		inline uint32_t GetNumIndices() { return m_NumIndices; }
		inline uint32_t GetNumVertices() { return m_NumVertices; }

		inline void SetPositions(std::vector<float>& vec) { m_Vertices = vec; }
		inline void SetNormals(std::vector<float>& vec) { m_Normals = vec; }
		inline void SetTangents(std::vector<float>& vec) { m_Tangents = vec; }
		inline void SetBitangents(std::vector<float>& vec) { m_Bitangents = vec; }
		inline void SetTexCoords(std::vector<float>& vec, uint32_t i) { m_TexCoords[i] = vec; }
		inline void SetIndices(std::vector<int>& vec) { m_Indices = vec; }
		inline void SetTransform(glm::mat4& transform) { m_Transform = transform; }

		inline bool HasColors() { return m_Colors.size() > 0; }
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
		uint32_t m_NumVertices;
		uint32_t m_NumIndices;
		uint32_t m_NumTexCoords = 0;

		std::string m_Name;
		std::string m_Path;
		std::string m_MetaPath;

		std::unordered_map<std::string, Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<VertexArray> m_VertexArray;

		std::vector<float> m_Vertices;
		std::vector<float> m_Colors;
		std::vector<float> m_Normals;
		std::vector<float> m_Tangents;
		std::vector<float> m_Bitangents;
		std::vector<std::vector<float>> m_TexCoords;
		std::vector<int> m_Indices;

		glm::mat4 m_Transform = glm::mat4(1.0f);
	};
}