#include "Debut/dbtpch.h"
#include "Entity.h"

#include "Components.h"

namespace Debut
{
	Entity::Entity(entt::entity handle, Scene* scene) 
		: m_EntityHandle(handle), m_Scene(scene)
	{
		
	}

	TransformComponent& Entity::Transform()
	{
		return GetComponent<TransformComponent>();
	}
}