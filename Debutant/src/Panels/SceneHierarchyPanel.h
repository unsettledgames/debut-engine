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

	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};
}
