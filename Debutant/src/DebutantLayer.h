#pragma once
#include <Debut.h>
#include <imgui.h>
#include "ImGuizmo.h"

#include <Debut/Renderer/EditorCamera.h>
#include <Debut/Scene/Entity.h>

#include <Panels/SceneHierarchyPanel.h>

namespace Debut
{
	class DebutantLayer : public Layer
	{
	public:
		DebutantLayer() : Layer("DebutantLayer") {}
		virtual ~DebutantLayer() {}

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender() override;

		void NewScene();
		void OpenScene();
		void SaveScene();
		void SaveSceneAs();

		bool OnKeyPressed(KeyPressedEvent& e);

	private:
		// Viewport data
		glm::vec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused;
		bool m_ViewportHovered;

		Ref<FrameBuffer> m_FrameBuffer;

		// Scene
		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;

		// Camera
		EditorCamera m_EditorCamera;

		// Panels
		SceneHierarchyPanel m_SceneHierarchy;

		// Editor state
		std::string m_ScenePath = "";

		// Gizmos
		ImGuizmo::OPERATION m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	};
}