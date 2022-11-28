#pragma once

#include <Debut/Scene/Scene.h>
#include <Debut/Core/UUID.h>
#include <Debut/Core/Log.h>
#include <entt.hpp>
#include <vector>
#include <xhash>

/*
		- Store transform tree?
*           - Create association between existing tree and transform tree?
*           - Directly use TransformComponents? No, probably a struct of <TransformComponent, glm::mat4>
*           - Wait wait wait, requirements:
*               - Translation, rotation, scale-> Really? Can't I just save the entity id and retrieve them only when necessary? In
*                   that way I'd avoid some redundancies
*               - NeedsUpdate
*               - TransformMatrix
*           - How does it work? Whenever you set one of the first 3 components, NeedsUpdate is set. Whenever the matrix is
*               required, before getting it, check if it NeedsUpdate. If so, recalculate it, cache it and send it to the
*               requester.
*           - That's great. Now, how do I easily access the data from the tree? Ideally it'd be neat to access it in O(1). What
				if std::unordered_map<EntityID, TransformNode>? Every time I need a transform matrix, I get it from
				the TransformNode by accessing the map using the entity ID should I use the entt id? In the end it's just a
				uint32_t, the rest of the engine doesn't need to know about entt.
			- Where do I save that map? Static member of TransformComponent? In that way it wouldn't take up space in each
				component while still being easily accessible from every point in which a TransformComponent is needed.
				The fact is that, once a transform matrix is updated, the children need an update too. The TransformNode should
				probably contain them as well, in order to propagate the transformations.
			- Cool idea, recycle and expand the EntitySceneNode. Make it so that it belongs to a scene and not to the
				SceneHierarchyPanel.
*/

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
			if (HasComponent<IDComponent>())
				component.Owner = GetComponent<IDComponent>().ID;
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
		bool operator== (const Entity& other) const 
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; 
		};
		bool operator!= (const Entity& other) const 
		{ 
			return m_EntityHandle != other.m_EntityHandle || m_Scene != other.m_Scene; 
		};
	
	public:
		static std::unordered_map<UUID, Entity> s_ExistingEntities;

	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
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

