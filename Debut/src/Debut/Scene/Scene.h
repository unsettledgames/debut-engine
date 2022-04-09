#pragma once

#include <entt.hpp>
#include "Components.h"
#include "Debut/Core/Time.h"

namespace Debut
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		void OnUpdate(Timestep ts);

		entt::entity CreateEntity();

		//TMP
		entt::registry& Reg() { return m_Registry; }

	private:
		entt::registry m_Registry;
	};
}

