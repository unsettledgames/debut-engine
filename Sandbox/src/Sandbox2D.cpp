#include "Sandbox2D.h"
#include <Debut/dbtpch.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

void Sandbox2D::OnAttach()
{
	m_Texture = Debut::Texture2D::Create("C:/dev/Debut/Debut/assets/textures/checkerboard.png");
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Debut::Timestep ts)
{
	static float rotation = 0.0f;
	//rotation += 500 * ts;

	m_CameraController.OnUpdate(ts);

	{
		DBT_PROFILE_SCOPE("Sandbox2D::RendererSetup");

		Debut::RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.2, 1));
		Debut::RenderCommand::Clear();

		Debut::Renderer2D::BeginScene(m_CameraController.GetCamera()/*camera, lights, environment*/);
	}
	
	{
		DBT_PROFILE_SCOPE("Sandbox2D::Rendering");

		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				if ((i + j) % 3 > 0)
					Debut::Renderer2D::DrawQuad(glm::vec3(i, j, -0.1), glm::vec2(1, 1), i+j+ rotation, (i + j) % 2 == 0 ? glm::vec4(0.2, 0.8, 0.2, 1) : glm::vec4(0.8, 0.2, 0.2, 1));
				else
					Debut::Renderer2D::DrawQuad(glm::vec3(i, j, -0.1), glm::vec2(1, 1), 45+ rotation, m_Texture);
			}
		}

		Debut::Renderer2D::EndScene();
		
	}

	Debut::Log.AppInfo("Frame time: %f", (1.0f / ts));
}

void Sandbox2D::OnEvent(Debut::Event& e)
{
	m_CameraController.OnEvent(e);
}

void Sandbox2D::OnImGuiRender()
{
	DBT_PROFILE_FUNCTION();
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Triangle color: ", glm::value_ptr(m_TriangleColor));
	ImGui::End();
}