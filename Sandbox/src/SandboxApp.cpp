#include <iostream>
#include <Debut/dbtpch.h>
#include <Debut.h>

class ExampleLayer : public Debut::Layer
{
public:
	ExampleLayer() : Layer("Example") {}

	void OnUpdate() override
	{
		Debut::Log.AppInfo("%s layer update", m_DebugName.c_str());
	}

	void OnEvent(Debut::Event& e) override
	{
		Debut::Log.AppInfo("%s layer event: %s", m_DebugName.c_str(), e.ToString().c_str());
	}
};

class Sandbox : public Debut::Application
{ 
public:
	Sandbox()
	{
		int a = 0;

		ExampleLayer* layer = new ExampleLayer();

		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}
private:

};

namespace Debut
{
	Application* CreateApplication()
	{
		return new Sandbox();
	}
}
