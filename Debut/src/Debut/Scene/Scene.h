#pragma once

#include <entt.hpp>
#include <vector>

#include <Debut/Core/Core.h>
#include <Debut/Rendering/Camera.h>
#include <Debut/Core/UUID.h>
#include "Debut/Core/Time.h"

class b2World;

namespace Debut
{
	class EntitySceneNode;

	class Scene
	{
	friend class Entity;
	friend class SceneHierarchyPanel;
	friend class TopPanel;
	friend class SceneSerializer;

	public:
		Scene();
		~Scene();

		void OnRuntimeUpdate(Timestep ts);
		void OnEditorUpdate(Timestep ts, Camera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		void OnRuntimeStart();
		void OnRuntimeStop();

		Entity CreateEntity(Entity parent, const std::string& name = "New Entity");
		Entity CreateEntity(Entity parent, const UUID& id, const std::string& name = "New Entity");
		void DuplicateEntity(Entity& entity);
		void DestroyEntity(Entity entity);

		Entity GetPrimaryCameraEntity();
		Entity GetEntityByID(uint64_t id);

		static Ref<Scene> Copy(Ref<Scene> other);
		void RebuildSceneGraph();

	private:
		template<typename T>
		void OnComponentAdded(T& component, Entity entity);

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		
		// Physics
		b2World* m_PhysicsWorld2D = nullptr;

		// Editor
		EntitySceneNode* m_CachedSceneGraph;
	};
}

