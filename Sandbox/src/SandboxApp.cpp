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
private:

};


Debut::Application* Debut::CreateApplication()
{
	return new Sandbox();
}