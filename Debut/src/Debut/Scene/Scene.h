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
	struct LightComponent;
	struct ShaderUniform;

	class Camera;
	class FrameBuffer;
	class Skybox;
	class PhysicsSystem3D;
	class ShadowMap;

	class Scene
	{
	friend class Entity;
	friend class SceneHierarchyPanel;
	// TEMPORARY
	friend class ViewportPanel;
	friend class TopPanel;
	friend class SceneSerializer;

	public:
		Scene();
		~Scene();

		void OnRuntimeUpdate(Timestep ts, Ref<FrameBuffer> target);
		void OnEditorUpdate(Timestep ts, Camera& camera, Ref<FrameBuffer> target);
		void OnViewportResize(uint32_t width, uint32_t height);

		void OnRuntimeStart();
		void OnRuntimeStop();
		
		void RenderingSetup(Ref<FrameBuffer> target);
		void Rendering2D(Camera& camera, const glm::mat4& cameraTransform, Ref<FrameBuffer> target);
		void Rendering3D(Camera& camera, const glm::mat4& cameraTransform, Ref<FrameBuffer> target);
		void RenderingDebug(Camera& camera, const glm::mat4& cameraTransform, Ref<FrameBuffer> target);

		Entity CreateEmptyEntity();
		Entity CreateEntity(Entity parent, const std::string& name = "New Entity");
		Entity CreateEntity(Entity parent, const UUID& id, const std::string& name = "New Entity");
		Entity DuplicateEntity(Entity& entity, Entity& parent);
		void DestroyEntity(Entity entity);

		Entity GetPrimaryCameraEntity();
		Entity GetEntityByID(uint64_t id);
		inline Ref<Skybox> GetSkybox() { return m_Skybox; }
		inline glm::vec3 GetAmbientLight() { return m_AmbientLight; }
		inline float GetAmbientLightIntensity() { return m_AmbientLightIntensity; }
		inline glm::vec2 GetViewportSize() { return { m_ViewportWidth, m_ViewportHeight }; }
		inline std::vector<Ref<ShadowMap>> GetShadowMaps() { return m_ShadowMaps; }

		void SetSkybox(UUID path);
		inline void SetAmbientLight(glm::vec3 light) { m_AmbientLight = light; }
		inline void SetAmbientLightIntensity(float light) { m_AmbientLightIntensity = light; }

		static Ref<Scene> Copy(Ref<Scene> other);
		std::vector<ShaderUniform> GetGlobalUniforms(glm::vec3 cameraPos);
		std::vector<LightComponent*> GetLights();

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

		std::vector<Ref<ShadowMap>> m_ShadowMaps;
		// TEMPORARY
		float lambda = 1.0f;
		float fadeoutStartDistance = 50;
		float fadeoutEndDistance = 70;
	};
}

