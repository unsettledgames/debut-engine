#pragma once

#include <Debut/Scene/Scene.h>
#include <entt.hpp>

namespace Debut
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& t) = default;

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			DBT_ASSERT(!HasComponent<T>(), "This entity already has the component you want to attach");
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			DBT_ASSERT(HasComponent<T>(), "This entity doesn't have the required component");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			DBT_ASSERT(HasComponent<T>(), "This entity doesn't have the required component");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		operator bool() const { return (uint32_t)m_EntityHandle != entt::null; }

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene;
	};
}

