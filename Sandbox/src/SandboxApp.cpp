#include <iostream>
#include <Debut/dbtpch.h>
#include <Debut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

class ExampleLayer : public Debut::Layer
{
public:
	ExampleLayer() : Layer("Example"), m_Camera(-1.6, 1.6f, -0.9f, 0.9f) 
	{
		float vertices[7 * 3] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};
		float textureVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 
			0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		};

		int indices[3] = { 0, 1, 2 };
		int textIndices[6] = { 0, 1, 2, 0, 2, 3};

		Debut::Ref<Debut::VertexBuffer> textBuffer;
		Debut::Ref<Debut::IndexBuffer> textIndBuffer;
		
		textBuffer = Debut::VertexBuffer::Create(textureVertices, 5 * 4);
		textIndBuffer = Debut::IndexBuffer::Create(textIndices, 6);
		m_TextureVA = Debut::VertexArray::Create();

		Debut::Ref<Debut::VertexBuffer> vertexBuffer;
		Debut::Ref<Debut::IndexBuffer> indexBuffer;

		vertexBuffer = Debut::VertexBuffer::Create(vertices, 3 * 7);
		indexBuffer = Debut::IndexBuffer::Create(indices, 3);
		m_VertexArray = Debut::VertexArray::Create();

		Debut::BufferLayout bufferLayout = { 
			{ Debut::ShaderDataType::Float3, "a_Position", false},
			{Debut::ShaderDataType::Float4, "a_Color", false}
		};

		Debut::BufferLayout squareLayout = {
			{Debut::ShaderDataType::Float3, "a_Position", false},
			{Debut::ShaderDataType::Float2, "a_UV", false}
		};

		vertexBuffer->SetLayout(bufferLayout);
		textBuffer->SetLayout(squareLayout);

		m_VertexArray->AddVertexBuffer(vertexBuffer);
		m_VertexArray->AddIndexBuffer(indexBuffer);

		m_TextureVA->AddVertexBuffer(textBuffer);
		m_TextureVA->AddIndexBuffer(textIndBuffer);
		 
		m_ShaderLibrary.Load("Unlit", "C:/dev/Debut/Debut/assets/shaders/unlit.glsl");
		m_ShaderLibrary.Load("Texture", "C:/dev/Debut/Debut/assets/shaders/texture.glsl");

		m_Texture = Debut::Texture2D::Create("C:/dev/Debut/Debut/assets/textures/akita.png");
		m_ShaderLibrary.Get("Texture")->Bind();
		std::dynamic_pointer_cast<Debut::OpenGLShader>(m_ShaderLibrary.Get("Texture"))->UploadUniformInt("u_Texture", 0);

		m_CameraPosition = glm::vec3(0, 0, 0);
	}

	void OnUpdate(Debut::Timestep ts) override
	{
		Debut::Log.AppInfo("Delta time: %f", (float)ts);

		if (Debut::Input::IsKeyPressed(DBT_KEY_LEFT))
			m_CameraPosition.x -= m_CameraMovementSpeed * ts;
		if (Debut::Input::IsKeyPressed(DBT_KEY_RIGHT))
			m_CameraPosition.x += m_CameraMovementSpeed * ts;

		if (Debut::Input::IsKeyPressed(DBT_KEY_DOWN))
			m_CameraPosition.y -= m_CameraMovementSpeed * ts;
		if (Debut::Input::IsKeyPressed(DBT_KEY_UP))
			m_CameraPosition.y += m_CameraMovementSpeed * ts;

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Debut::RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.2, 1));
		Debut::RenderCommand::Clear();

		Debut::Renderer::BeginScene(m_Camera/*camera, lights, environment*/);

		glm::vec3 startPos(0.0f);
		static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				std::dynamic_pointer_cast<Debut::OpenGLShader>(m_ShaderLibrary.Get("Unlit"))->UploadUniformFloat4("u_Color", m_TriangleColor);
				m_TriangleColor.a = 1.0f;

				startPos = glm::vec3(i * 0.1f, j * 0.1f, 0.0f);
				Debut::Renderer::Submit(m_VertexArray, m_ShaderLibrary.Get("Unlit"), glm::translate(glm::mat4(1.0f), startPos) * scale);
			}
		}
		
		m_Texture->Bind();
		Debut::Renderer::Submit(m_TextureVA, m_ShaderLibrary.Get("Texture"), glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)));
		

		Debut::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Triangle color: ", glm::value_ptr(m_TriangleColor));
		ImGui::End();
	}

	void OnEvent(Debut::Event& e) override
	{
		Debut::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Debut::KeyPressedEvent>(DBT_BIND(ExampleLayer::OnKeyPressed));
	}

	bool OnKeyPressed(Debut::KeyPressedEvent& e)
	{

		return false;
	}
private:
	Debut::ShaderLibrary m_ShaderLibrary;
	Debut::Ref<Debut::Texture2D> m_Texture;

	Debut::Ref<Debut::VertexArray> m_VertexArray;
	Debut::Ref<Debut::VertexArray> m_TextureVA;
	
	Debut::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMovementSpeed = 1;
	float m_CameraRotation = 0;
	float m_CameraRotationSpeed = 40;

	glm::vec4 m_TriangleColor;
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
