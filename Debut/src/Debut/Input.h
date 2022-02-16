#pragma once

#include "Debut/Core.h"

namespace Debut
{
	class DBT_API Input
	{
	public:
		static bool IsKeyPressed(unsigned int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }

	protected:
		virtual bool IsKeyPressedImpl(unsigned int keycode) const = 0;
	private:
		static Input* s_Instance;
	};
}