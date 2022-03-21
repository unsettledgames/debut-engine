#include "Debut/dbtpch.h"
#include "Debut/Renderer/RenderCommand.h"
#include "Debut/Renderer/VertexArray.h"
#include "Debut/Renderer/Shader.h"
#include "Renderer2D.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Debut
{
	struct Renderer2DStorage
	{
		Ref<VertexArray> VertexArray;
		Ref<Shader> Shader;
	};

	static Renderer2DStorage* s_Data;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DStorage();
		s_Data->VertexArray = VertexArray::Create();

		float textureVertices[3 * 4] = {
			-0.5f, -0.5f, 0.0f,
			-0.5f, 0.5f, 0.0f,
			0.5f, 0.5f, 0.0f,
			0.5f, -0.5f, 0.0f
		};
		int textIndices[6] = { 0, 1, 2, 0, 2, 3 };

		Ref<VertexBuffer> textBuffer = VertexBuffer::Create(textureVertices, 3 * 4);
		Ref<IndexBuffer> textIndBuffer = IndexBuffer::Create(textIndices, 6);
		s_Data->VertexArray = VertexArray::Create();

		BufferLayout squareLayout = {
				{ShaderDataType::Float3, "a_Position", false}
		};

		textBuffer->SetLayout(squareLayout);
		s_Data->VertexArray->AddVertexBuffer(textBuffer);
		s_Data->VertexArray->AddIndexBuffer(textIndBuffer);

		s_Data->Shader = Shader::Create("C:/dev/Debut/Debut/assets/shaders/unlit.glsl");
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		s_Data->Shader->Bind();
		s_Data->Shader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer2D::EndScene()
	{

	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4 color)
	{
		Renderer2D::DrawQuad(glm::vec3(position, 0.0f), size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4 color)
	{
		s_Data->Shader->Bind();
		s_Data->Shader->SetFloat4("u_Color", color);

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * 
			glm::scale(glm::mat4(1.0f), glm::vec3(size, 0.0f));
		s_Data->Shader->SetMat4("u_Transform", transform);

		s_Data->VertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->VertexArray);
	}
}