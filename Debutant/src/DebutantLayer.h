#pragma once
#include <Debut.h>
#include <imgui.h>
#include "ImGuizmo.h"
#include <Debut/Scene/Entity.h>

#include <Panels/SceneHierarchyPanel.h>

namespace Debut
{
	class DebutantLayer : public Layer
	{
	public:
		DebutantLayer() : Layer("DebutantLayer"), m_CameraController(1920.0f / 1080.0f, true) {}
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
		OrthographicCameraController m_CameraController;

		glm::vec2 m_ViewportSize;
		bool m_ViewportFocused;
		bool m_ViewportHovered;

		// Kinda placeholderish
		Ref<Texture2D> m_Texture;
		Ref<Texture2D> m_Checkerboard;
		Ref<SubTexture2D> m_BushTexture;

		Ref<FrameBuffer> m_FrameBuffer;

		// Scene
		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_Camera;

		// Panels
		SceneHierarchyPanel m_SceneHierarchy;

		// Editor state
		std::string m_ScenePath = "";

		// Gizmos
		ImGuizmo::OPERATION m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	};
}