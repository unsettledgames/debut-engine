#include <iostream>
// Entry point
#include <DebutantApp.h>
#include "Debut/Core/EntryPoint.h"
#include <Debut.h>
#include <Scene/SceneManager.h>
#include "DebutantLayer.h"


namespace Debut
{
	DebutantApp::DebutantApp() : Application("Debutant")
	{
		PushLayer(new DebutantLayer());
	}

	Application* CreateApplication()
	{
		return new DebutantApp();
	}

}