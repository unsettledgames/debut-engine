#include <iostream>
#include <Debut/dbtpch.h>
#include <Debut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"

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
		int indices[3] = { 0, 1, 2 };

		std::shared_ptr<Debut::VertexBuffer> vertexBuffer;
		std::shared_ptr<Debut::IndexBuffer> indexBuffer;

		vertexBuffer.reset(Debut::VertexBuffer::Create(vertices, 3 * 7));
		indexBuffer.reset(Debut::IndexBuffer::Create(indices, 3));
		m_VertexArray.reset(Debut::VertexArray::Create());

		Debut::BufferLayout bufferLayout = {
			{ Debut::ShaderDataType::Float3, "a_Position", false},
			{Debut::ShaderDataType::Float4, "a_Color", false}
		};

		vertexBuffer->SetLayout(bufferLayout);

		m_VertexArray->AddVertexBuffer(vertexBuffer);
		m_VertexArray->AddIndexBuffer(indexBuffer);

		std::string vertSrc = R"(
			#version 410
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragSrc = R"(
			#version 410
			
			layout(location = 0) out vec4 color;

			in vec4 v_Color;

			void main()
			{
				color = v_Color;
			}
		)";

		m_Shader.reset(new Debut::Shader(vertSrc, fragSrc));

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

		m_Shader->Bind();
		glm::vec3 startPos(0.0f);
		static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				startPos = glm::vec3(i * 0.1f, j * 0.1f, 0.0f);
				Debut::Renderer::Submit(m_VertexArray, m_Shader, glm::translate(glm::mat4(1.0f), startPos) * scale);
			}
		}
		

		Debut::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
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
	std::shared_ptr<Debut::Shader> m_Shader;
	std::shared_ptr<Debut::VertexArray> m_VertexArray;
	
	Debut::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMovementSpeed = 1;
	float m_CameraRotation = 0;
	float m_CameraRotationSpeed = 40;
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
