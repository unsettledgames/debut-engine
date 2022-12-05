#include <Platform/OpenGL/OpenGLRenderTexture.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>
#include <Platform/OpenGL/OpenGLError.h>
#include <glad/glad.h>

namespace Debut
{
	OpenGLRenderTexture::OpenGLRenderTexture(float width, float height, Ref<FrameBuffer> buffer, RenderTextureMode mode)
	{
		m_Width = width;
		m_Height = height;
		m_Mode = mode;

		float vertices[16] = { 
			-1, 1, 0, 1,
			1, 1, 1, 1,
			1, -1, 1, 0,
			-1, -1, 0, 0};
		int indices[6] = { 0, 2, 1, 2, 0, 3 };

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

	void OpenGLRenderTexture::Bind()
	{
		uint32_t attachment = 0;

		switch (m_Mode)
		{
		case RenderTextureMode::Color:
			attachment = m_FrameBuffer->GetColorAttachment();
			break;
		case RenderTextureMode::Depth:
			attachment = m_FrameBuffer->GetDepthAttachment();
			break;
		}

		GLCall(glBindTextureUnit(0, attachment));
	}

	void OpenGLRenderTexture::Unbind()
	{
		GLCall(glBindTextureUnit(0, 0));
	}
}