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

		std::string vertSrc = R"(
			#version 410
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragSrc = R"(
			#version 410
			
			layout(location = 0) out vec4 color;

			uniform vec4 u_Color;

			void main()
			{
				color = u_Color;
			}
		)";

		std::string squareVert = R"(
			#version 410
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_UV;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec2 v_UV;

			void main()
			{
				v_UV = a_UV;

				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string squareFrag = R"(
			#version 410
			
			layout(location = 0) out vec4 color;
			layout(location = 1) in vec2 v_UV;

			uniform sampler2D u_Texture;

			void main()
			{
				color = texture(u_Texture, v_UV);
			}
		)";

		m_Shader = Debut::Shader::Create(vertSrc, fragSrc);
		m_SquareShader = Debut::Shader::Create(squareVert, squareFrag);

		m_Texture = Debut::Texture2D::Create("C:/dev/Debut/Debut/assets/textures/penguin.png");
		m_SquareShader->Bind();
		std::dynamic_pointer_cast<Debut::OpenGLShader>(m_SquareShader)->UploadUniformInt("u_Texture", 0);

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
				m_Shader->Bind();
				std::dynamic_pointer_cast<Debut::OpenGLShader>(m_Shader)->UploadUniformFloat4("u_Color", m_TriangleColor);

				startPos = glm::vec3(i * 0.1f, j * 0.1f, 0.0f);
				Debut::Renderer::Submit(m_VertexArray, m_Shader, glm::translate(glm::mat4(1.0f), startPos) * scale);
			}
		}
		
		m_Texture->Bind();
		Debut::Renderer::Submit(m_TextureVA, m_SquareShader, glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)));
		

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
	Debut::Ref<Debut::Texture2D> m_Texture;
	Debut::Ref<Debut::Shader> m_Shader;
	Debut::Ref<Debut::Shader> m_SquareShader;
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
