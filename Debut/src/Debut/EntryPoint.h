#pragma once

#include "dbtpch.h"
#include "Debut.h"

#ifdef DBT_PLATFORM_WINDOWS

extern Debut::Application* Debut::CreateApplication();

int main(int argc, char** argv)
{
	// Initialize the logging system
	Debut::Log.Init();
	// Create the application
	auto app = Debut::CreateApplication();
	Debut::Log.CoreInfo("Created application");
	
	app->Run();

	delete app;

	return 0;
}

#endif