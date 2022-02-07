#include <iostream>
#include <Debut.h>

class Sandbox : public Debut::Application
{ 
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}

	void Run () override
	{
		Debut::Log::CoreInfo();
		Debut::Log::CoreInfo();
		Debut::Log::CoreInfo();
		Debut::Log::CoreInfo();

		while (true)
		{

		}
	}
private:

};

Debut::Application* Debut::CreateApplication()
{
	return new Sandbox();
}