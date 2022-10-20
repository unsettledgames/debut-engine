#include "Debut/dbtpch.h"
#include "Entity.h"

#include "Components.h"

namespace Debut
{
	std::unordered_map<UUID, Entity> Entity::s_ExistingEntities;

	Entity::Entity(entt::entity handle, Scene* scene) 
		: m_EntityHandle(handle), m_Scene(scene)
	{
		
	}

	TransformComponent& Entity::Transform()
	{
		return GetComponent<TransformComponent>();
	}

	uint64_t Entity::ID()
	{
		return GetComponent<IDComponent>().ID;
	}
}