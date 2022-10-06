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
	struct ShaderUniform;

	class Camera;
	class Skybox;
	class PhysicsSystem3D;

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
		inline Ref<Skybox> GetSkybox() { return m_Skybox; }
		inline glm::vec3 GetAmbientLight() { return m_AmbientLight; }
		inline float GetAmbientLightIntensity() { return m_AmbientLightIntensity; }

		void SetSkybox(UUID path);
		inline void SetAmbientLight(glm::vec3 light) { m_AmbientLight = light; }
		inline void SetAmbientLightIntensity(float light) { m_AmbientLightIntensity = light; }

		static Ref<Scene> Copy(Ref<Scene> other);
		std::vector<ShaderUniform> GetGlobalUniforms(glm::vec3 cameraPos);

	private:
		template<typename T>
		void OnComponentAdded(T& component, Entity entity);

	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		
		// Physics
		b2World* m_PhysicsWorld2D = nullptr;		
		PhysicsSystem3D* m_PhysicsSystem3D = nullptr;

		// Lighting
		Ref<Skybox> m_Skybox;
		glm::vec3 m_AmbientLight = glm::vec3(0.0f);
		float m_AmbientLightIntensity = 1.0f;
	};
}

