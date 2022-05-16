#pragma once

#include "Debut.h"

namespace Debut
{
	class SceneHierarchyPanel
	{

	public:
		SceneHierarchyPanel() = default;

		void SetContext(const Ref<Scene>& scene);
		void SetSelectedEntity(const Entity& entity);

		void OnImGuiRender();

		Entity GetSelectionContext() const { return m_SelectionContext; }

	private:
		void DrawEntityNode(Entity& entity);
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
	};
}
