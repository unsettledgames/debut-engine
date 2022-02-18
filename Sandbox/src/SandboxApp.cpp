#include <iostream>
#include <Debut/dbtpch.h>
#include <Debut.h>
#include <glm/glm.hpp>
#include "imgui.h"

class ExampleLayer : public Debut::Layer
{
public:
	ExampleLayer() : Layer("Example") {}

	void OnUpdate() override
	{
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::Text("Test text");
		ImGui::End();
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
