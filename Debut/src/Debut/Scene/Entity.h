#pragma once

#include <Debut/Scene/Scene.h>
#include <Debut/Core/UUID.h>
#include <entt.hpp>
#include <vector>
#include <xhash>

namespace Debut
{
	struct TransformComponent;

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
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(component, *this);
			return component;
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(component, *this);
			return component;
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

		inline bool IsValid() { return m_Scene->m_Registry.valid(m_EntityHandle); }

		TransformComponent& Transform();
		uint64_t ID();

		operator bool() const { return (uint32_t)m_EntityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }
		bool operator== (const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; };
		bool operator!= (const Entity& other) const { return m_EntityHandle != other.m_EntityHandle || m_Scene == other.m_Scene; };

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene;
	};

	struct EntitySceneNode
	{
		bool IsRoot = false;
		Entity EntityData;
		uint32_t IndexInNode = -1;
		std::vector<EntitySceneNode*> Children;

		EntitySceneNode(bool root, Entity entity) : IsRoot(root), EntityData(entity), Children({}) {}
		EntitySceneNode() : EntityData({}), Children({}) {}

		~EntitySceneNode()
		{
		}
	};
}

