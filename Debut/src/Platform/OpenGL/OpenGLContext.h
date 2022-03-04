#pragma once

#include "Debut/Renderer/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace Debut
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle;
	};
}

