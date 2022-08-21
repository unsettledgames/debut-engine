#include <iostream>
// Entry point
#include "Debut/Core/EntryPoint.h"
#include <Debut.h>
#include "DebutantLayer.h"


namespace Debut
{
	class DebutantApp : public Application
	{
	public:
		DebutantApp() : Application("Debutant")
		{
			PushLayer(new DebutantLayer());
		}

		~DebutantApp()
		{

		}
	private:

	};

	Application* CreateApplication()
	{
		return new DebutantApp();
	}

}