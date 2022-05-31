#pragma once

#include <Debut/Core/UUID.h>
#include <Debut/Rendering/Structures/VertexArray.h>

namespace Debut
{
	class Mesh
	{
	public:
		Mesh();
		Mesh(const std::string& path) {};

		UUID GetID() { return m_ID; }
		
	private:
		UUID m_ID;
		Ref<VertexArray> m_VertexArray;
	};
}