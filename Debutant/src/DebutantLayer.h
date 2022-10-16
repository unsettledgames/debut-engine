#pragma once
#include <Debut.h>
#include <imgui.h>
#include "ImGuizmo.h"

#include <Panels/SceneHierarchyPanel.h>
#include <Panels/ContentBrowserPanel.h>
#include <Camera/EditorCamera.h>
#include <Debut/Scene/Components.h>

namespace Debut
{
	struct PhysicsColliderSelection
	{
		Entity SelectedEntity;
		glm::vec3 SelectedPoint = {0,0,0};
		glm::mat4 PointTransform = glm::mat4(1.0);
		glm::mat4 PointRotation = glm::mat4(1.0);
		std::string SelectedName = "";

		bool Valid = false;
	};

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
		virtual void OnUpdate(Timestep& ts) override;
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

	private:
		glm::vec4 GetHoveredPixel(uint32_t attachmentIndex);
		glm::vec2 GetFrameBufferCoords();

		// UI panels
		void DrawUIToolbar();
		void DrawViewport();
		void DrawTopBar();
		void DrawSettingsWindow();
		void DrawAssetMapWindow();

		// Debug & Gizmos
		void DrawTransformGizmos();
		void DrawPhysicsGizmos();
		void ManipulatePhysicsGizmos();

		// Drag & droppable objects
		void LoadModel(const std::filesystem::path path);
		void LoadModelNode(Ref<Model> model, Entity& parent);

	private:

		// Viewport data
		glm::vec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused;
		bool m_ViewportHovered;

		Ref<FrameBuffer> m_FrameBuffer;

		// Scene
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene, m_RuntimeScene;
		// Camera
		EditorCamera m_EditorCamera;

		// Panels
		SceneHierarchyPanel m_SceneHierarchy;
		ContentBrowserPanel m_ContentBrowser;
		PropertiesPanel m_PropertiesPanel;

		// Editor state
		std::string m_ScenePath = "";
		Entity m_HoveredEntity;
		SceneState m_SceneState = SceneState::Edit;

		bool m_AssetMapOpen = false;
		bool m_SettingsOpen = false;

		uint32_t m_StartIndex = -1;
		uint32_t m_EndIndex = -1;

		// Textures
		AssetCache<std::string, Ref<Texture2D>> m_TextureCache;
		Ref<Texture2D> m_IconPlay;
		Ref<Texture2D> m_IconStop;

		// Gizmos
		ImGuizmo::OPERATION m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
		PhysicsColliderSelection m_PhysicsSelection;
	};
}