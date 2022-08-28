#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Core/Time.h>
#include <Debut/Core/LayerStack.h>
#include "Debut/Events/ApplicationEvent.h"

#include <string>


namespace Debut
{
	class ApplicationEvent;
	class WindowCloseEvent;
	class WindowResizedEvent;

	class Layer;
	class ImGuiLayer;
	class Window;

	class Application
	{
	public:
		Application(const std::string& name = "Debut Application");
		
		virtual ~Application();

		void Run();
		void Close();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		inline Window& GetWindow() { return *m_Window; }
		static inline Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResized(WindowResizedEvent& e);

	private:
		bool m_Running = true;
		bool m_Minimized = false;

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;

		LayerStack m_LayerStack;
		static Application* s_Instance;

		float m_LastFrameTime = 0.0f;
		Timestep m_Timestep;
	};

	Application* CreateApplication();
}
