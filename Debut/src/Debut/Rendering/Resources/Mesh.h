#pragma once

#include <Debut/Core/UUID.h>
#include <Debut/Rendering/Structures/Frustum.h>
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

		void SaveSettings(std::vector<float>& positions, std::vector<float>& colors, std::vector<float>& normals, 
			std::vector<float>& tangents, std::vector<float>& bitangents, std::vector<std::vector<float>>& texcoords,
			std::vector<int>& indices);

		inline UUID GetID() { return m_ID; }
		inline std::string GetName() { return m_Name; }
		inline std::string GetPath() { return m_Path; }
		inline bool IsValid() { return m_Valid; }
		inline std::vector<float>& GetPositions() { return m_Vertices; }
		inline std::vector<int>& GetIndices() { return m_Indices; }

		inline Ref<VertexArray> GetVertexArray() { return m_VertexArray; }
		inline glm::mat4& GetTransform() { return m_Transform; }
		inline uint32_t GetNumIndices() { return m_NumIndices; }
		inline uint32_t GetNumVertices() { return m_NumVertices; }
		inline AABB GetAABB() { return m_AABB; }

		inline void SetPositions(const std::vector<float>& vertices) { m_Vertices = vertices; }
		inline void SetIndices(const std::vector<int>& indices) { m_Indices = indices; }
		inline void SetTransform(glm::mat4& transform) { m_Transform = transform; }
		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetPath(const std::string& path) { m_Path = path; }
		inline void SetID(const UUID& id) { m_ID = id; }

		void GenerateAABB(const std::vector<float>& vertices);
		void GenerateBuffers();
		void Load(const std::string& path);

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
		std::vector<int> m_Indices;

		glm::mat4 m_Transform = glm::mat4(1.0f);
		AABB m_AABB;
	};
}