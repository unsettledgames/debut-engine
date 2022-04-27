#pragma once

#include <entt.hpp>
#include "Debut/Core/Time.h"

namespace Debut
{
	class Scene
	{
	friend class Entity;
	friend class SceneHierarchyPanel;
	friend class TopPanel;
	friend class SceneSerializer;

	public:
		Scene();
		~Scene();

		void OnUpdate(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity CreateEntity(const std::string& name = "New Entity");
		void DestroyEntity(Entity entity);

		Entity GetPrimaryCameraEntity();

	private:
		template<typename T>
		void OnComponentAdded(T& component, Entity entity);

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
	};
}

