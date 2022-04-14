#include "Debut/dbtpch.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include "Debut/Scene/Entity.h"
#include "Debut/Renderer/Renderer2D.h"

namespace Debut
{
	Scene::Scene()
	{
		
	}

	Scene::~Scene()
	{

	}

	void Scene::OnUpdate(Timestep ts)
	{
		// Render sprites

		// Find the main camera of the scene
		Camera* mainCamera = nullptr;
		glm::mat4* cameraTransform;
		{
			auto view = m_Registry.view<CameraComponent, TransformComponent>();
			for (auto entity : view)
			{
				auto& [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
				if (camera.Primary)
				{
					mainCamera = &(camera.Camera);
					cameraTransform = &(transform.Transform);
					break;
				}
			}
		}

		if (mainCamera)
		{
			DBT_PROFILE_SCOPE("Renderer2D update");
			Renderer2D::BeginScene(mainCamera, *cameraTransform);


			auto group = m_Registry.group<TransformComponent, SpriteRendererComponent>();
			for (auto entity : group)
			{
				auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
				Renderer2D::DrawQuad(transform, sprite.Color);
			}

			Renderer2D::EndScene();
		}
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity ret = { m_Registry.create(), this };
		
		ret.AddComponent<TransformComponent>();
		ret.AddComponent<TagComponent>("New Entity");

		return ret;
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
	}
}