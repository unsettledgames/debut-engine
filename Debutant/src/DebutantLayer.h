#pragma once
#include <Debut.h>
#include <imgui.h>
#include "ImGuizmo.h"

#include <Debut/Renderer/EditorCamera.h>
#include <Debut/Scene/Entity.h>
#include <Debut/Events/MouseEvent.h>
#include <Panels/SceneHierarchyPanel.h>
#include <Panels/ContentBrowserPanel.h>

namespace Debut
{
	class DebutantLayer : public Layer
	{
	public:
		enum class SceneState
		{
			Edit = 0, Play = 1
		};

		DebutantLayer() : Layer("DebutantLayer") {}
		virtual ~DebutantLayer() {}

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender() override;

		void NewScene();
		void OpenScene();
		void OpenScene(std::filesystem::path path);
		void SaveScene();
		void SaveSceneAs();

		void OnScenePlay();
		void OnSceneStop();

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		// UI panels
		void DrawUIToolbar();
		void DrawGizmos();
		void DrawViewport();
		void DrawTopBar();

	private:
		// Viewport data
		glm::vec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused;
		bool m_ViewportHovered;

		Ref<FrameBuffer> m_FrameBuffer;

		// Scene
		Ref<Scene> m_ActiveScene;
		// Camera
		EditorCamera m_EditorCamera;

		// Panels
		SceneHierarchyPanel m_SceneHierarchy;
		ContentBrowserPanel m_ContentBrowser;

		// Editor state
		std::string m_ScenePath = "";
		Entity m_HoveredEntity;
		SceneState m_SceneState = SceneState::Edit;

		// Textures
		Ref<Texture2D> m_IconPlay;
		Ref<Texture2D> m_IconStop;

		// Gizmos
		ImGuizmo::OPERATION m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
	};
}