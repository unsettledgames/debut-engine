#include "Debut/dbtpch.h"
#include "Application.h"
#include "Input.h"

#include "Log.h"
#include <glad/glad.h>

namespace Debut
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		DBT_ASSERT(!s_Instance, "Application already exists.");

		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(DBT_BIND(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		m_LayerStack.PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		while (m_Running)
		{
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			// Propagate update to the stack
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			// Render ImGui
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(DBT_BIND(Application::OnWindowClosed));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			// Propagate the event until it's not handled
			(*--it)->OnEvent(e);
			if (e.Handled())
				break;
		}
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}
}