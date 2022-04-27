#include "Debut/dbtpch.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include "Debut/Scene/Entity.h"
#include "Debut/Scene/Components.h"
#include "Debut/Renderer/Renderer2D.h"

namespace Debut
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{

	}

	template<typename T>
	void Scene::OnComponentAdded(T& component, Entity entity)
	{
		static_assert(false);
	}

	template<>
	void Scene::OnComponentAdded(TransformComponent& component, Entity entity) {}
	template<>
	void Scene::OnComponentAdded(SpriteRendererComponent& sr, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(TagComponent& tc, Entity entity) { }
	template<>
	void Scene::OnComponentAdded(NativeScriptComponent& nsc, Entity entity) { }

	template<>
	void Scene::OnComponentAdded(CameraComponent& camera, Entity entity)
	{
		camera.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto e : view)
		{
			const auto& camera = view.get<CameraComponent>(e);
			if (camera.Primary)
				return Entity{ e, this };
		}
		return {};
	}
	

	void Scene::OnUpdate(Timestep ts)
	{
		// Update scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
				{
					if (!nsc.Instance)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->m_Entity = { entity, this };
						nsc.Instance->OnCreate();
					}

					nsc.Instance->OnUpdate(ts);
				});
		}

		// Render sprites

		// Find the main camera of the scene
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<CameraComponent, TransformComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
				if (camera.Primary)
				{
					mainCamera = &(camera.Camera);
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			DBT_PROFILE_SCOPE("Renderer2D update");
			Renderer2D::BeginScene(mainCamera, cameraTransform);

			auto group = m_Registry.group<TransformComponent, SpriteRendererComponent>();
			for (auto entity : group)
			{
				auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
				Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
			}

			Renderer2D::EndScene();
		}
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity ret = { m_Registry.create(), this };
		
		ret.AddComponent<TransformComponent>();
		ret.AddComponent<TagComponent>(name);

		return ret;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto& entity : view)
		{
			CameraComponent& camera = view.get<CameraComponent>(entity);
			if (!camera.FixedAspectRatio)
				camera.Camera.SetViewportSize(width, height);
		}

		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}
}