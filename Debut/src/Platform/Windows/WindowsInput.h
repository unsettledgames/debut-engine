#pragma once

#include "Debut/Input.h"
#include "Debut/Core.h"

namespace Debut
{
	class DBT_API WindowsInput : public Input
	{
	protected:
		virtual bool IsKeyPressedImpl(unsigned int keycode) override;
		virtual bool IsMouseButtonPressedImpl(unsigned int button) override;
		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}