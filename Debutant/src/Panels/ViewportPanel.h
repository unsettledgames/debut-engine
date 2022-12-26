#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Scene/Entity.h>

#include <Camera/EditorCamera.h>
#include <Widgets/Gizmos.h>

#include <glm/glm.hpp>

struct ImVec2;

namespace Debut
{
	class Scene;
	class FrameBuffer;
	class RenderTexture;
	class DebutantLayer;
	class Shader;

	class Timestep;
	class KeyPressedEvent;
	class MouseButtonPressedEvent;

	class ViewportPanel
	{
	public:
		ViewportPanel() = default;
		ViewportPanel(DebutantLayer* layer);

		void OnUpdate(Timestep& ts);
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void OnImGuiRender();
		void DrawTopBar(ImVec2& menuSize);
		void DrawCollider(std::vector<glm::vec3>& points, std::vector<std::string>& labels);
		void SelectCollider(std::vector<glm::vec3> points, std::vector<std::string> labels);

		glm::vec2 GetFrameBufferCoords();

		inline bool IsFocused() { return m_ViewportFocused; }
		inline glm::vec2 GetViewportSize() { return m_ViewportSize; }
		inline glm::vec2* GetViewportBounds() { return m_ViewportBounds; }
		inline glm::vec2 GetMenuSize() { return m_TopMenuSize; }
		inline Ref<FrameBuffer> GetFrameBuffer() { return m_SceneFrameBuffer; }
		inline Entity GetSelectedEntity() { return m_Selection; }

		inline EditorCamera& Camera() { return m_EditorCamera; }

		void SetSelectedEntity(Entity entity) { m_Selection = entity; }

	private:
		// Viewport data
		glm::vec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2];
		glm::vec2 m_TopMenuSize;

		// Viewport state
		bool m_ViewportFocused;
		bool m_ViewportHovered;

		// Camera
		EditorCamera m_EditorCamera;
		// Gizmos
		Gizmos m_Gizmos;

		// Selection
		Entity m_Selection;
		PhysicsColliderSelection m_ColliderSelection;

		DebutantLayer* m_ParentLayer;
		// Scene rendering
		Ref<FrameBuffer> m_SceneFrameBuffer;
		Ref<FrameBuffer> m_DebugFrameBuffer;
		Ref<RenderTexture> m_RenderTexture;

		Ref<Shader> m_FullscreenShader;
		Ref<Shader> m_TranparentFullscreenShader;
		float fps;
	};
}