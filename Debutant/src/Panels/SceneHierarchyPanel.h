#pragma once

#include <imgui.h>
#include <vector>
#include <Debut/Core/Core.h>
#include <Debut/Scene/Entity.h>

namespace Debut
{
	class Scene;
	class InspectorPanel;

	struct TransformComponent;

	class SceneHierarchyPanel
	{

	public:
		SceneHierarchyPanel();
		~SceneHierarchyPanel();

		void SetContext(const Ref<Scene>& scene);
		void SetInspectorPanel(InspectorPanel* panel) { m_Inspector = panel; }
		inline void SetSelectedEntity(const Entity& entity)
		{
			if ((int)(entt::entity)entity != -1)
				m_SelectionContext = entity;
			else
				m_SelectionContext = {};
		}

		void OnImGuiRender();
		void DrawEntityNode(EntitySceneNode& entity);

		inline Entity GetSelectionContext() { return m_SelectionContext; }
		inline EntitySceneNode* GetSceneGraph() { return m_CachedSceneGraph; }

		void RebuildSceneGraph();
		void Reset();
		void LoadTree(EntitySceneNode* node);
		void ChangeEntityOrder(uint32_t movedEntity, int position, uint32_t newParent = -1);

	private:
		uint32_t GetParentInSceneGraph(EntitySceneNode* node, uint32_t entity);
		void DestroySceneNode(EntitySceneNode& node);

	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
		InspectorPanel* m_Inspector;

		// Scene graph management
		bool m_RebuiltGraph = false;
		EntitySceneNode* m_CachedSceneGraph;
		std::unordered_map<entt::entity, EntitySceneNode*> m_ExistingEntities;
		// Key is child of value
		std::unordered_map<uint32_t, uint32_t> m_EntityParenting;

		// State
		Entity m_LastHoveredEntity = {};
		ImVec2 m_LastMousePos = {};
		ImVec2 m_LastHoveredMousePos = {};
		ImVec2 m_LastHoveredItemPos = {};
		ImVec2 m_LastItemSize = {};

		bool m_DroppedOnEntity = false;
		bool m_DraggingEntity = false;
		bool m_HoveringEntity = false;
		bool m_LastHoveredOpen = false;
	};
}
