#include "Sandbox2D.h"
#include <Debut/dbtpch.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

template<typename Fn>
class Timer
{
public:
	Timer(const char* name, Fn&& func) : m_Name(name), m_Stopped(false), m_Func(func)
	{
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		if (!m_Stopped)
			Stop();
	}

	void Stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		m_Stopped = true;
		m_Func({ m_Name, (float)(end - start) * 0.001f });
	}

private:
	const char* m_Name;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
	bool m_Stopped;
	Fn m_Func;
};

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) {m_ProfileResults.push_back(profileResult);})

void Sandbox2D::OnAttach()
{
	m_Texture = Debut::Texture2D::Create("C:/dev/Debut/Debut/assets/textures/checkerboard.png");
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Debut::Timestep ts)
{
	PROFILE_SCOPE("Sandbox2D::OnUpdate");
	m_CameraController.OnUpdate(ts);

	{
		PROFILE_SCOPE("Sandbox2D::RendererSetup");

		Debut::RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.2, 1));
		Debut::RenderCommand::Clear();

		Debut::Renderer2D::BeginScene(m_CameraController.GetCamera()/*camera, lights, environment*/);
	}
	
	{
		PROFILE_SCOPE("Sandbox2D::Rendering");

		Debut::Renderer2D::DrawQuad(glm::vec2(0, -1), glm::vec2(1, 1), glm::vec4(0.2, 0.8, 0.2, 1));
		Debut::Renderer2D::DrawQuad(glm::vec2(1, 0), glm::vec2(1, 1), glm::vec4(0.8, 0.2, 0.2, 1));

		Debut::Renderer2D::DrawQuad(glm::vec3(0, 0, -0.1), glm::vec2(10, 10), m_Texture);

		Debut::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnEvent(Debut::Event& e)
{
	m_CameraController.OnEvent(e);
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Triangle color: ", glm::value_ptr(m_TriangleColor));

	// Profile info
	for (auto& result : m_ProfileResults)
	{
		char label[50];
		strcpy(label, "%.3fms ");
		strcat(label, result.Name);
		
		ImGui::Text(label, result.Time);
	}
	m_ProfileResults.clear();

	ImGui::End();
}