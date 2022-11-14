#include "Debut/dbtpch.h"
#include "Debut/Core/Log.h"
#include "WindowsWindow.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include "Debut/Events/KeyEvent.h"
#include "Debut/Events/MouseEvent.h"
#include "Debut/Events/ApplicationEvent.h"
#include <glad/glad.h>

namespace Debut
{
	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int errCode, const char* errMsg)
	{
		Debut::Log.CoreError("Error {0}: {1}", errCode, errMsg);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props) 
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		DBT_PROFILE_FUNCTION();
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		DBT_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		

		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			DBT_ASSERT(success, "Couldn't initialize GLFW");

			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		m_Context = new OpenGLContext(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(false);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				data.Width = width;
				data.Height = height;

				WindowResizedEvent wndwEvent(width, height);
				data.EventCallback(wndwEvent);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent clsEvent;

				data.EventCallback(clsEvent);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent prsdEvent(key, 0);
						data.EventCallback(prsdEvent);

						break;
					}
					case GLFW_RELEASE:
					{
						KeyReleasedEvent rlsdEvent(key);
						data.EventCallback(rlsdEvent);

						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent prsdEvent(key, 1);
						data.EventCallback(prsdEvent);

						break;
					}
				}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				KeyTypedEvent e(keycode);

				data.EventCallback(e);
			});	 

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent prsdEvent(button);
						data.EventCallback(prsdEvent);

						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent rlsdEvent(button);
						data.EventCallback(rlsdEvent);

						break;
					}
				}
			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseScrolledEvent scrollEvent((float)xOffset, (float)yOffset);
				data.EventCallback(scrollEvent);
			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseMovedEvent moveEvent((float)xPos, (float)yPos);
				data.EventCallback(moveEvent);
			});
	}

	Window* Window::Create(const WindowProps& props)
	{
		Debut::Log.CoreInfo("Created window {0}: {1}, {2}", props.Title.c_str(), props.Width, props.Height);
		return new WindowsWindow(props);
	}

	void WindowsWindow::OnUpdate() const
	{
		DBT_PROFILE_FUNCTION();
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::Shutdown()
	{
		DBT_PROFILE_FUNCTION();
		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		DBT_PROFILE_FUNCTION();
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
}