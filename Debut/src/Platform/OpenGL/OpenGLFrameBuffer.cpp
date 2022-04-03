#include "Debut/dbtpch.h"
#include "OpenGLFrameBuffer.h"
#include "OpenGLError.h"
#include "glad/glad.h"

namespace Debut
{
	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecs& specs) : m_Specs(specs)
	{
		Invalidate();
	}

	void OpenGLFrameBuffer::Invalidate()
	{
		GLCall(glCreateFramebuffers(1, &m_RendererID));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));

		GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_ColorAttachment));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specs.Width, m_Specs.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0));

		// Depth buffer
		GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_DepthAttachment));
		GLCall(glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Specs.Width, m_Specs.Height));
		//GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specs.Width, m_Specs.Height, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL));
		
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0));
		DBT_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Frame buffer is incomplete");
		
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
	}

	void OpenGLFrameBuffer::Bind() const
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));
	}

	void OpenGLFrameBuffer::Unbind() const
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

		
}