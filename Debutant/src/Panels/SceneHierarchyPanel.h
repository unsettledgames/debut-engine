#pragma once

#include "Debut.h"
#include <vector>


namespace Debut
{
	class SceneHierarchyPanel
	{

	public:
		SceneHierarchyPanel();

		void SetContext(const Ref<Scene>& scene);
		void SetSelectedEntity(const Entity& entity);

		void OnImGuiRender();

		inline Entity GetSelectionContext() { 
			if (m_SelectionContext)
			{
				TransformComponent& transform = m_SelectionContext.Transform();
				int sas = 5;
			}
			return m_SelectionContext; 
		}

	private:
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
		bool m_RebuiltGraph = false;
	};
}
