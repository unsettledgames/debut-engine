#pragma once

#include "Debut.h"
#include <imgui.h>
#include <vector>


namespace Debut
{
	class SceneHierarchyPanel
	{

	public:
		SceneHierarchyPanel();
		~SceneHierarchyPanel();

		void SetContext(const Ref<Scene>& scene);
		void SetSelectedEntity(const Entity& entity);
		void RegisterEntity(const Entity& entity);

		void OnImGuiRender();

		inline Entity GetSelectionContext() { 
			if (m_SelectionContext)
			{
				TransformComponent& transform = m_SelectionContext.Transform();
				int sas = 5;
			}
			return m_SelectionContext; 
		}

		inline EntitySceneNode* GetSceneGraph() { return m_CachedSceneGraph; }

		void RebuildSceneGraph();
		void ChangeEntityOrder(uint32_t movedEntity, uint32_t position);

	private:
		uint32_t GetParentInSceneGraph(EntitySceneNode* node, uint32_t entity);

		void DrawEntityNode(EntitySceneNode& entity);
		void DrawComponents(Entity& entity);

		template <typename T>
		void DrawAddComponentEntry(const std::string& componentName)
		{
			if (!m_SelectionContext.HasComponent<T>())
			{
				if (ImGui::MenuItem(componentName.c_str()))
				{
					m_SelectionContext.AddComponent<T>();
					ImGui::CloseCurrentPopup();
				}
			}
		}

		void DestroySceneNode(EntitySceneNode& node);

	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;

		// Scene graph management
		bool m_RebuiltGraph = false;
		EntitySceneNode* m_CachedSceneGraph;
		std::unordered_map<entt::entity, EntitySceneNode*> m_ExistingEntities;
		std::unordered_map<entt::entity, uint32_t> m_EntitiesOrdering;
		// Key is child of value
		std::unordered_map<uint32_t, uint32_t> m_EntityParenting;

		// State
		// remove
		bool m_HoveringInvisibleEntityButton = false;
		// keep
		Entity m_LastHoveredEntity = {};
		ImVec2 m_LastHoveredPos = {};

		bool m_DroppedOnEntity = false;
		bool m_DraggingEntity = false;
	};
}
