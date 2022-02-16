#pragma once

#include "Debut/Events/ApplicationEvent.h"
#include "Debut/Events/MouseEvent.h"
#include "Debut/Events/KeyEvent.h"
#include "Debut/Layer.h"

namespace Debut
{
	class DBT_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach();
		void OnDetach();
		void OnUpdate();
		void OnEvent(Event& e);
	private:
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
		bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);
		bool OnMouseScroll(MouseScrolledEvent& e);

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnKeyReleased(KeyReleasedEvent& e);
		bool OnKeyTyped(KeyTypedEvent& e);

		bool OnWindowResized(WindowResizedEvent& e);
		bool OnWindowMoved(WindowMovedEvent& e);
	private:
		float m_Time = 0;
	};
}