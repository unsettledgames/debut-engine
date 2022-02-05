#pragma once

#include "Core.h"

namespace Debut
{
	class DBT_API Application
	{
	public:
		Application();
		
		virtual ~Application();

		void Run();
	};

	Application* CreateApplication();
}
