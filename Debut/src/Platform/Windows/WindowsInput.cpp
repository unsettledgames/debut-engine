#include "Debut/dbtpch.h"
#include "WindowsInput.h"
#include "Debut/Application.h"
#include "GLFW/glfw3.h"

namespace Debut
{
	bool WindowsInput::IsKeyPressedImpl(unsigned int keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
}