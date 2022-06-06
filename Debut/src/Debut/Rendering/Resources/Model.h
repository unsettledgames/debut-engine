#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>

namespace Debut
{
	class Model
	{
	public:
		Model(std::vector<UUID> meshes, std::vector<UUID> materials, std::vector<UUID> subModels) :
			m_Meshes(meshes), m_Materials(materials), m_SubModels(subModels) {}

		UUID GetID() { return m_ID; }

	private:
		UUID m_ID;
		std::vector<UUID> m_Materials;
		std::vector<UUID> m_Meshes;

		std::vector<UUID> m_SubModels;
	};
}