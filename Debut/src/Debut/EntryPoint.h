#pragma once

#include "Debut.h"

#ifdef DBT_PLATFORM_WINDOWS

extern Debut::Application* Debut::CreateApplication();

int main(int argc, char** argv)
{
	// Initialize the logging system
	Debut::Log.Init();
	// Create the application
	auto app = Debut::CreateApplication();

	DBT_INFO("TRY\n");
	DBT_CORE_INFO("TRY 2\n");
	
	app->Run();

	Debut::Log.Shutdown();

	delete app;

	return 0;
}

#endif