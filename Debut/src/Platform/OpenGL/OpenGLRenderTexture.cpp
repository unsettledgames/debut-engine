#include <Platform/OpenGL/OpenGLRenderTexture.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/Renderer/RenderCommand.h>

#include <Platform/OpenGL/OpenGLError.h>
#include <glad/glad.h>

namespace Debut
{
	OpenGLRenderTexture::OpenGLRenderTexture(float width, float height, Ref<FrameBuffer> buffer)
	{
		m_Width = width;
		m_Height = height;

		float vertices[16] = { 
			-1, 1, 0, 1,
			1, 1, 1, 1,
			1, -1, 1, 0,
			-1, -1, 0, 0};
		int indices[6] = { 0, 1, 2, 2, 3, 0 };

		m_VertexBuffer = VertexBuffer::Create(vertices, 16);
		m_IndexBuffer = IndexBuffer::Create(indices, 6);
		m_VertexArray = VertexArray::Create();
		m_FrameBuffer = buffer;

		m_VertexBuffer->SetLayout(BufferLayout({ 
			{ ShaderDataType::Float2, "a_Position", false },
			{ ShaderDataType::Float2, "a_TexCoords", false}
		}));

		m_VertexArray->AddIndexBuffer(m_IndexBuffer);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);
	}

	void OpenGLRenderTexture::Draw(Ref<Shader> shader)
	{
		m_VertexArray->Bind();
		shader->Bind();
		shader->SetInt("u_Texture", 0);
		Bind();

		RenderCommand::DrawIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());
		
		Unbind();
		shader->Unbind();
		m_VertexArray->Unbind();
	}

	void OpenGLRenderTexture::Bind()
	{
		GLCall(glBindTexture(GL_TEXTURE_2D, m_FrameBuffer->GetColorAttachment()));
	}

	void OpenGLRenderTexture::Unbind()
	{
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	}
}