#include "Debut/dbtpch.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include "Debut/Renderer/Renderer2D.h"

namespace Debut
{
	static void DoStuff(const glm::mat4& color)
	{

	}

	Scene::Scene()
	{
		
	}

	Scene::~Scene()
	{

	}

	void Scene::OnUpdate(Timestep ts)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			Renderer2D::DrawQuad(transform, sprite.Color);
		}
	}

	entt::entity Scene::CreateEntity()
	{
		return m_Registry.create();
	}
}