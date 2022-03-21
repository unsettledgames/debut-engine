#include "Sandbox2D.h"
#include <Debut/dbtpch.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/matrix_transform.hpp>

void Sandbox2D::OnAttach()
{
	float textureVertices[3 * 4] = {
			-0.5f, -0.5f, 0.0f, 
			-0.5f, 0.5f, 0.0f, 
			0.5f, 0.5f, 0.0f, 
			0.5f, -0.5f, 0.0f
	};
	int textIndices[6] = { 0, 1, 2, 0, 2, 3 };

	Debut::Ref<Debut::VertexBuffer> textBuffer = Debut::VertexBuffer::Create(textureVertices, 3 * 4);
	Debut::Ref<Debut::IndexBuffer> textIndBuffer = Debut::IndexBuffer::Create(textIndices, 6);
	m_TextureVA = Debut::VertexArray::Create();

	Debut::BufferLayout squareLayout = {
			{Debut::ShaderDataType::Float3, "a_Position", false}
	};

	textBuffer->SetLayout(squareLayout);
	m_TextureVA->AddVertexBuffer(textBuffer);
	m_TextureVA->AddIndexBuffer(textIndBuffer);

	m_ShaderLibrary.Load("Unlit", "C:/dev/Debut/Debut/assets/shaders/unlit.glsl");
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Debut::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	Debut::RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.2, 1));
	Debut::RenderCommand::Clear();

	Debut::Renderer::BeginScene(m_CameraController.GetCamera()/*camera, lights, environment*/);

	glm::vec3 startPos(0.0f);

	m_ShaderLibrary.Get("Unlit")->Bind();
	std::dynamic_pointer_cast<Debut::OpenGLShader>(m_ShaderLibrary.Get("Unlit"))->UploadUniformFloat4("u_Color", m_TriangleColor);

	Debut::Renderer::Submit(m_TextureVA, m_ShaderLibrary.Get("Unlit"), glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)));

	Debut::Renderer::EndScene();
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