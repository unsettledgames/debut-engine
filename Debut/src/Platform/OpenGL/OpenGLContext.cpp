#include "Debut/dbtpch.h"
#include "OpenGLContext.h"
#include "Debut/Core/Core.h"
#include "Debut/Core/Log.h"

#include <glad/glad.h>

namespace Debut
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {}

	void OpenGLContext::Init()
	{
		DBT_PROFILE_FUNCTION();
		glfwMakeContextCurrent(m_WindowHandle);

		int success = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		DBT_ASSERT(success, "Failed to initialize Glad");

		Log.CoreInfo("OPENGL INFO:");
		Log.CoreInfo("OpenGL renderer: {0}", glGetString(GL_RENDERER));
		Log.CoreInfo("OpenGL version: {0}", glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{
		DBT_PROFILE_FUNCTION();
		glfwSwapBuffers(m_WindowHandle);
	}
}