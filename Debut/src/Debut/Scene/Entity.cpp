#include "Debut/dbtpch.h"
#include "Entity.h"

namespace Debut
{
	Entity::Entity(entt::entity handle, Scene* scene) 
		: m_EntityHandle(handle), m_Scene(scene)
	{
		
	}
}