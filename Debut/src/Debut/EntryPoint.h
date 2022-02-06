#pragma once
#include <stdio.h>

#ifdef DBT_PLATFORM_WINDOWS

extern Debut::Application* Debut::CreateApplication();

int main(int argc, char** argv)
{
	// Create the application
	auto app = Debut::CreateApplication();
	
	// Initialize the logging system
	Debut::Log::Init();
	
	app->Run();

	delete app;

	return 0;
}

#endif