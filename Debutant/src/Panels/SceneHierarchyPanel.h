#pragma once

#include "Debut.h"
#include <vector>


namespace Debut
{
	struct SceneNode
	{
		bool IsRoot = false;
		Entity ParentEntity;
		std::vector<SceneNode> Children;

		SceneNode(bool root, Entity parent) : IsRoot(root), ParentEntity(parent), Children({}) {}
		SceneNode() : ParentEntity({}), Children({}) {}
	};

	class SceneHierarchyPanel
	{

	public:
		SceneHierarchyPanel();

		void SetContext(const Ref<Scene>& scene);
		void SetSelectedEntity(const Entity& entity);
		SceneNode RebuildSceneGraph();

		void OnImGuiRender();

		Entity GetSelectionContext() const { return m_SelectionContext; }

	private:
		void DrawEntityNode(SceneNode& entity);
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

	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
		SceneNode m_CachedSceneGraph;
		bool m_RebuiltGraph = false;
	};
}
