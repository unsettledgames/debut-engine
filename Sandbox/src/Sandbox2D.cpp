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
    Debut::FrameBufferSpecs fbSpecs;

    fbSpecs.Width = Debut::Application::Get().GetWindow().GetWidth();
    fbSpecs.Height = Debut::Application::Get().GetWindow().GetHeight();

	m_Texture = Debut::Texture2D::Create("C:/dev/Debut/Sandbox/assets/tileset.png");
    m_Checkerboard = Debut::Texture2D::Create("C:/dev/Debut/Debut/assets/textures/checkerboard.png");
	m_CameraController.SetZoomLevel(2);
	m_BushTexture = Debut::SubTexture2D::CreateFromCoords(m_Texture, glm::vec2(0, 4), glm::vec2(5, 6), glm::vec2(16, 16));
    m_FrameBuffer = Debut::FrameBuffer::Create(fbSpecs);

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
        m_FrameBuffer->Bind();

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

	Debut::Log.AppInfo("Frame time: %f", (1.0f / ts));
    m_FrameBuffer->Unbind();
}

void Sandbox2D::OnEvent(Debut::Event& e)
{
	m_CameraController.OnEvent(e);
}

void Sandbox2D::OnImGuiRender()
{
	DBT_PROFILE_FUNCTION();
	auto stats = Debut::Renderer2D::GetStats();

    static bool dockspaceOpen = true;
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) Debut::Application::Get().Close();
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Triangle color: ", glm::value_ptr(m_TriangleColor));

    // Renderer2D stats
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw calls: %d", stats.DrawCalls);
    ImGui::Text("Quads: %d", stats.QuadCount);
    ImGui::Text("Vertex count: %d", stats.GetTotalVertexCount());
    ImGui::Text("Index count: %d", stats.GetIndexCount());

    uint32_t texId = m_FrameBuffer->GetColorAttachment();
    ImGui::Image((void*)texId, ImVec2(320, 180), ImVec2{ 0,1 }, ImVec2{ 1,0 });

    ImGui::End();

    ImGui::End();
}