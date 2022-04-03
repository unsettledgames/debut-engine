#include <iostream>
// Entry point
#include "Debut/Core/EntryPoint.h"
#include <Debut.h>
#include "DebutantLayer.h"


namespace Debut
{
	class Debutant : public Application
	{
	public:
		Debutant()
		{
			PushLayer(new DebutantLayer());
		}

		~Debutant()
		{

		}
	private:

	};

	Application* CreateApplication()
	{
		return new Debutant();
	}

}