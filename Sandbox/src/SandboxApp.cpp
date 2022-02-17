#include <iostream>
#include <Debut/dbtpch.h>
#include <Debut.h>
#include <glm/glm.hpp>

class ExampleLayer : public Debut::Layer
{
public:
	ExampleLayer() : Layer("Example") {}

	void OnUpdate() override
	{
	}

	void OnEvent(Debut::Event& e) override
	{
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
		PushOverlay(new Debut::ImGuiLayer());

		glm::vec2(45.0f, 23.0f);
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
