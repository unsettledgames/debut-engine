#pragma once

#include <Debut/Core/Core.h>
#include <glm/glm.hpp>

struct ImVec2;

namespace Debut
{
	class Scene;
	class FrameBuffer;
	class DebutantLayer;

	class ViewportPanel
	{
	public:
		ViewportPanel() = default;
		ViewportPanel(DebutantLayer* layer, Ref<FrameBuffer> frameBuffer) : m_ParentLayer(layer), m_FrameBuffer(frameBuffer) {}

		void OnImGuiRender();
		void DrawTopBar(ImVec2& menuSize);

		inline void SetContext(Scene* context) { m_Context = context; }

		inline bool IsFocused() { return m_ViewportFocused; }
		inline glm::vec2 GetViewportSize() { return m_ViewportSize; }
		inline glm::vec2* GetViewportBounds() { return m_ViewportBounds; }
		inline glm::vec2 GetMenuSize() { return m_TopMenuSize; }

	private:
		// Viewport data
		glm::vec2 m_ViewportSize;
		glm::vec2 m_ViewportBounds[2];
		glm::vec2 m_TopMenuSize;

		// Viewport state
		bool m_ViewportFocused;
		bool m_ViewportHovered;

		Scene* m_Context;
		DebutantLayer* m_ParentLayer;
		Ref<FrameBuffer> m_FrameBuffer;
	};
}