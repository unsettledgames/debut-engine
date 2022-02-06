#include <stdio.h>
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
		DBT_INFO("Game started\n");
	}
private:

};

Debut::Application* Debut::CreateApplication()
{
	return new Sandbox();
}