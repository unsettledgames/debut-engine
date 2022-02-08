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
		DBT_INFO("Info {0}", 87);

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