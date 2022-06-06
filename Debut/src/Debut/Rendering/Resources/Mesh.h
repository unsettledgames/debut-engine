#pragma once

#include <Debut/Core/UUID.h>
#include <Debut/Rendering/Structures/VertexArray.h>

namespace Debut
{
	struct MeshVertex;

	class Mesh
	{
	public:
		Mesh();
		Mesh(const std::string& path) {};

		UUID GetID() { return m_ID; }
		
	private:
		UUID m_ID;
		std::vector<MeshVertex> m_Vertices;
	};
}