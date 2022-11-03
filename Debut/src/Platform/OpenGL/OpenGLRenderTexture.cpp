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

		float vertices[8] = { -1, -1, 1, -1, 1, 1, -1, -1 };
		int indices[6] = { 0, 1, 2, 2, 3, 1 };

		m_VertexBuffer = VertexBuffer::Create(vertices, 8);
		m_IndexBuffer = IndexBuffer::Create(indices, 6);
		m_VertexArray = VertexArray::Create();

		m_VertexBuffer->SetLayout(BufferLayout({ { ShaderDataType::Float3, "a_Position", false } }));

		m_VertexArray->AddIndexBuffer(m_IndexBuffer);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);

		// generate texture
		GLCall(glGenTextures(1, &m_RendererID));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));

		buffer->Bind();
		// attach it to currently bound framebuffer object
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + buffer->GetSpecs().Attachments.Attachments.size(), 
			GL_TEXTURE_2D, m_RendererID, 0));
		buffer->Unbind();
	}

	void OpenGLRenderTexture::Draw(Ref<Shader> shader)
	{
		Bind();
		m_VertexArray->Bind();
		RenderCommand::DrawIndexed(m_VertexArray, m_VertexArray->GetIndexBuffer()->GetCount());
		m_VertexArray->Unbind();
		Unbind();
	}

	void OpenGLRenderTexture::Bind()
	{
		GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
	}

	void OpenGLRenderTexture::Unbind()
	{
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	}
}