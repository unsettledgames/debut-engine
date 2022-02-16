#pragma once

#include "Debut/Input.h"
#include "Debut/Core.h"

namespace Debut
{
	class DBT_API WindowsInput : public Input
	{
	protected:
		virtual bool IsKeyPressedImpl(unsigned int keycode) override;
	private:
	};
}