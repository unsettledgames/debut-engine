#include "Debut/dbtpch.h"
#include "GLFW/glfw3.h"
#include "Application.h"
#include "Input.h"

#include "Log.h"
#include "Debut/Renderer/RenderCommand.h"
#include "Debut/Renderer/Renderer.h"

namespace Debut
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		DBT_ASSERT(!s_Instance, "Application already exists.")

		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(DBT_BIND(Application::OnEvent));

		Renderer::Init();

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
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				// Propagate update to the stack
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(timestep);
			}
			
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
		dispatcher.Dispatch<WindowResizedEvent>(DBT_BIND(Application::OnWindowResized));

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

	bool Application::OnWindowResized(WindowResizedEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		Renderer::OnWindowResized(e.GetWidth(), e.GetHeight());
		m_Minimized = false;
		return false;
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