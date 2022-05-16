#pragma once

#include "Debut/Core/Core.h"

namespace Debut
{
	class Input
	{
	public:
		static bool IsKeyPressed(unsigned int keycode);
		static bool IsMouseButtonPressed(unsigned int button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}