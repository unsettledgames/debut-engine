#pragma once

#include "Debut/dbtpch.h"
#include "Debut.h"

#ifdef DBT_PLATFORM_WINDOWS

extern Debut::Application* Debut::CreateApplication();

int main(int argc, char** argv)
{
	Debut::Application* app;
	DBT_PROFILE_BEGIN_SESSION("Startup", "DebutProfile-Startup.json");
	{
		DBT_PROFILE_SCOPE("App initialization");
		// Initialize the logging system
		Debut::Log.Init();
		// Create the application
		app = Debut::CreateApplication();
		Debut::Log.CoreInfo("Created application");
	}
	DBT_PROFILE_END_SESSION();

	DBT_PROFILE_BEGIN_SESSION("Startup", "DebutProfile-Runtime.json");
	app->Run();
	DBT_PROFILE_END_SESSION();

	DBT_PROFILE_BEGIN_SESSION("Startup", "DebutProfile-Shutdown.json");
	delete app;
	DBT_PROFILE_END_SESSION();

	return 0;
}

#endif