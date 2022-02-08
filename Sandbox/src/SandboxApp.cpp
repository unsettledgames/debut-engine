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
		Debut::Log.AppInfo("Started running game");
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