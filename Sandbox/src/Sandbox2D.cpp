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

	m_Texture = Debut::Texture2D::Create("C:/dev/Debut/Sandbox/assets/tileset.png");
    m_Checkerboard = Debut::Texture2D::Create("C:/dev/Debut/Debut/assets/textures/checkerboard.png");
	m_CameraController.SetZoomLevel(2);
	m_BushTexture = Debut::SubTexture2D::CreateFromCoords(m_Texture, glm::vec2(0, 4), glm::vec2(5, 6), glm::vec2(16, 16));

	m_Particle.ColorBegin = glm::vec4( 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f );
	m_Particle.ColorEnd = glm::vec4(0 / 255.0f, 40 / 255.0f, 255 / 255.0f, 1.0f );
	m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 5.0f;
	m_Particle.Velocity = glm::vec2(0.0f, 0.0f );
	m_Particle.VelocityVariation = glm::vec2(4.0f, 4.0f );
	m_Particle.Position = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Debut::Timestep ts)
{
	m_CameraController.OnUpdate(ts);
	Debut::Renderer2D::ResetStats();
	{
		DBT_PROFILE_SCOPE("Sandbox2D::RendererSetup");

		Debut::RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.2, 1));
		Debut::RenderCommand::Clear();

		Debut::Renderer2D::BeginScene(m_CameraController.GetCamera()/*camera, lights, environment*/);
	}
	
	{
		DBT_PROFILE_SCOPE("Sandbox2D::Rendering");

		if (Debut::Input::IsMouseButtonPressed(DBT_MOUSE_BUTTON_LEFT))
		{
			auto [x, y] = Debut::Input::GetMousePosition();
			auto width = Debut::Application::Get().GetWindow().GetWidth();
			auto height = Debut::Application::Get().GetWindow().GetHeight();

			auto bounds = m_CameraController.GetBounds();
			auto pos = m_CameraController.GetCamera().GetPosition();
			x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
			y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
			m_Particle.Position = glm::vec3(x + pos.x, y + pos.y, 0);
			for (int i = 0; i < 5; i++)
				m_ParticleSystem.Emit(m_Particle);
		}

		m_ParticleSystem.OnUpdate(ts);
		m_ParticleSystem.OnRender(m_CameraController.GetCamera());

		Debut::Renderer2D::EndScene();
	}
	
	Debut::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Debut::Renderer2D::DrawQuad(glm::vec3(0, 0, 0.1), glm::vec2(10.0f, 10.0f), 0, m_Texture);
	Debut::Renderer2D::DrawQuad(glm::vec3(-10, 0, 0.1), glm::vec2(1, 1), 0, m_BushTexture);

	Debut::Renderer2D::EndScene();

	Debut::Log.AppInfo("Frame time: {0}", (1.0f / ts));
}

void Sandbox2D::OnEvent(Debut::Event& e)
{
	m_CameraController.OnEvent(e);
}

void Sandbox2D::OnImGuiRender()
{
	DBT_PROFILE_FUNCTION();
	auto stats = Debut::Renderer2D::GetStats();


    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Triangle color: ", glm::value_ptr(m_TriangleColor));

    // Renderer2D stats
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw calls: %d", stats.DrawCalls);
    ImGui::Text("Quads: %d", stats.QuadCount);
    ImGui::Text("Vertex count: %d", stats.GetTotalVertexCount());
    ImGui::Text("Index count: %d", stats.GetIndexCount());

    ImGui::End();
}