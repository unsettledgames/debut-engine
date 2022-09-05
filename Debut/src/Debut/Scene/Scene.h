#pragma once

#include <entt.hpp>
#include <glm/glm.hpp>
#include <vector>

#include <Debut/Core/Core.h>
#include <Debut/Core/UUID.h>
#include "Debut/Core/Time.h"

class b2World;

namespace Debut
{
	struct EntitySceneNode;
	class Camera;
	class Skybox;

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
		Ref<Skybox> GetSkybox() { return m_Skybox; }
		glm::vec3 GetAmbientLight() { return m_AmbientLight; }

		void SetSkybox(UUID path);
		void SetAmbientLight(glm::vec3 light) { m_AmbientLight = light; }

		static Ref<Scene> Copy(Ref<Scene> other);

	private:
		template<typename T>
		void OnComponentAdded(T& component, Entity entity);

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		
		// Physics
		b2World* m_PhysicsWorld2D = nullptr;		

		// Lighting
		Ref<Skybox> m_Skybox;
		glm::vec3 m_AmbientLight;
	};
}

