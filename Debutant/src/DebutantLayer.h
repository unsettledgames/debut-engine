#pragma once
#include <Debut.h>

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
		glm::vec4 m_TriangleColor = glm::vec4(0.3, 0.3, 1, 1);

		// Scene
		Ref<Scene> m_ActiveScene;
		entt::entity m_SquareEntity;
	};
}