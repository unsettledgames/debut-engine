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
		Ref<Shader> TextureShader;
		Ref<Texture> WhiteTexture;
	};

	static Renderer2DStorage* s_Data;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DStorage();
		s_Data->VertexArray = VertexArray::Create();

		float textureVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			-0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 0.0f
		};
		int textIndices[6] = { 0, 1, 2, 0, 2, 3 };

		Ref<VertexBuffer> textBuffer = VertexBuffer::Create(textureVertices, 5 * 4);
		Ref<IndexBuffer> textIndBuffer = IndexBuffer::Create(textIndices, 6);
		s_Data->VertexArray = VertexArray::Create();

		BufferLayout squareLayout = {
			{ShaderDataType::Float3, "a_Position", false},
			{ShaderDataType::Float2, "a_UV", false}
		};

		textBuffer->SetLayout(squareLayout);
		s_Data->VertexArray->AddVertexBuffer(textBuffer);
		s_Data->VertexArray->AddIndexBuffer(textIndBuffer);

		s_Data->TextureShader = Shader::Create("C:/dev/Debut/Debut/assets/shaders/texture.glsl");
		s_Data->TextureShader->SetInt("u_Texture", 0);

		s_Data->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t data = 0xffffffff;
		s_Data->WhiteTexture->SetData(&data, sizeof(data));
	}

	void Renderer2D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		s_Data->TextureShader->Bind();
		s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
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
		s_Data->TextureShader->SetFloat4("u_Color", color);
		s_Data->WhiteTexture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size, 0.0f));
		s_Data->TextureShader->SetMat4("u_Transform", transform);

		s_Data->VertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->VertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture>& texture)
	{
		Renderer2D::DrawQuad(glm::vec3(position, 0.0f), size, texture);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture>& texture)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size, 0.0f));

		s_Data->TextureShader->SetFloat4("u_Color", glm::vec4(1.0f));
		s_Data->TextureShader->SetMat4("u_Transform", transform);

		texture->Bind();

		s_Data->VertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->VertexArray);
	}
}