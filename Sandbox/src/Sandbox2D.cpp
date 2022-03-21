#include "Sandbox2D.h"
#include <Debut/dbtpch.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/matrix_transform.hpp>

void Sandbox2D::OnAttach()
{
	
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Debut::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	Debut::RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.2, 1));
	Debut::RenderCommand::Clear();

	Debut::Renderer2D::BeginScene(m_CameraController.GetCamera()/*camera, lights, environment*/);
	
	Debut::Renderer2D::DrawQuad(glm::vec2(0, 0), glm::vec2(1, 1), glm::vec4(0.2, 0.2, 0.8, 1));
	
	Debut::Renderer2D::DrawQuad(glm::vec2(0, -1.5), glm::vec2(2, 1), glm::vec4(0.2, 0.8, 0.2, 1));
	Debut::Renderer2D::DrawQuad(glm::vec2(1.5, 0), glm::vec2(1, 2), glm::vec4(0.8, 0.2, 0.2, 1));
	
	Debut::Renderer2D::EndScene();
}

void Sandbox2D::OnEvent(Debut::Event& e)
{
	m_CameraController.OnEvent(e);
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Triangle color: ", glm::value_ptr(m_TriangleColor));
	ImGui::End();
}