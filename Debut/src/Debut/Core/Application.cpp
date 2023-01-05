#include <Debut/dbtpch.h>
#include <GLFW/glfw3.h>

#include <Debut/Core/Application.h>
#include <Debut/Scripting/ScriptEngine.h>
#include <Debut/Core/Window.h>
#include <Debut/ImGui/ImGuiLayer.h>
#include <Debut/Core/Input.h>

#include <Debut/Rendering/Renderer/Renderer.h>

namespace Debut
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
	{
		DBT_PROFILE_FUNCTION();
		DBT_ASSERT(!s_Instance, "Application already exists.")

		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create(name));
		m_Window->SetEventCallback(DBT_BIND(Application::OnEvent));

		Renderer::Init();
		ScriptEngine::Init();

		m_ImGuiLayer = new ImGuiLayer();
		m_LayerStack.PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		DBT_PROFILE_SCOPE("Application::Run");
		{
			while (m_Running)
			{
				DBT_PROFILE_SCOPE("Application Loop Frame");

				float time = (float)glfwGetTime();
				Timestep timestep = time - m_LastFrameTime;
				m_LastFrameTime = time;

				if (!m_Minimized)
				{
					DBT_PROFILE_SCOPE("Layer updates")
					// Propagate update to the stack
					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(timestep);
				}
				
				{
					DBT_PROFILE_SCOPE("ImGui");
					// Render ImGui
					m_ImGuiLayer->Begin();
					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();
					m_ImGuiLayer->End();
				}

				{
					DBT_PROFILE_SCOPE("Window Update");
					m_Window->OnUpdate();
				}
				
			}
		}
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnEvent(Event& e)
	{
		DBT_PROFILE_FUNCTION();
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
		DBT_PROFILE_FUNCTION();
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