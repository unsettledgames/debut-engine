#include "Debut/dbtpch.h"
#include "OpenGLFrameBuffer.h"
#include "OpenGLError.h"
#include "glad/glad.h"

namespace Debut
{
	static const unsigned int MAX_FRAME_BUFFER_SIZE = 65535;

	namespace Utils
	{
		static GLenum TextureTarget(bool multiSamples)
		{
			return multiSamples ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static bool IsDepthFormat(FrameBufferTextureFormat format)
		{
			if (format == FrameBufferTextureFormat::DEPTH24STENCIL8)
				return true;
			return false;
		}

		static void BindTextures(bool multisampled, uint32_t id)
		{
			GLCall(glBindTexture(TextureTarget(multisampled), id));
		}

		static void CreateTextures(bool multiSamples, uint32_t* outID, uint32_t count)
		{
			GLCall(glCreateTextures(TextureTarget(multiSamples), count, outID));
		}

		static void AttachColorTexture(uint32_t texture, uint32_t nSamples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, uint32_t idx)
		{
			bool multisampled = nSamples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nSamples, internalFormat, width, height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

				// TODO: move to frame buffer specs
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, TextureTarget(multisampled), texture, 0);
		}

		static void AttachDepthTexture(uint32_t texture, uint32_t nSamples, GLenum format, GLenum attachmentType,
			uint32_t width, uint32_t height)
		{
			bool multisampled = nSamples > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nSamples, format, width, height, GL_FALSE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				// TODO: move to frame buffer specs
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}


			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), texture, 0);
		}
	}


	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecifications& specs) : m_Specs(specs)
	{
		for (auto spec : m_Specs.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(spec.TextureFormat))
				m_ColorAttachmentSpecs.emplace_back(spec);
			else
				m_DepthAttachmentSpecs = spec;
		}

		Invalidate();
	}

	void OpenGLFrameBuffer::Invalidate()
	{
		if (m_RendererID)
		{
			GLCall(glDeleteFramebuffers(1, &m_RendererID));
			GLCall(glDeleteTextures(m_ColorAttachmentSpecs.size(), m_ColorAttachments.data()));
			GLCall(glDeleteTextures(1, &m_DepthAttachment));

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		GLCall(glCreateFramebuffers(1, &m_RendererID));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));

		bool multiSamples = m_Specs.Samples > 1;
		
		if (m_ColorAttachmentSpecs.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
			Utils::CreateTextures(multiSamples, m_ColorAttachments.data(), m_ColorAttachmentSpecs.size());

			for (int i=0; i<m_ColorAttachmentSpecs.size(); i++)
			{
				Utils::BindTextures(multiSamples, m_ColorAttachments[i]);
				switch (m_ColorAttachmentSpecs[i].TextureFormat)
				{
				case FrameBufferTextureFormat::RGBA8:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specs.Samples, GL_RGBA8, GL_RGBA, m_Specs.Width, m_Specs.Height, i);
					break;
				case FrameBufferTextureFormat::RED_INTEGER:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specs.Samples, GL_R32I, GL_RED_INTEGER, m_Specs.Width, m_Specs.Height, i);
					break;
				default:
					DBT_CORE_ASSERT(false, "Frame buffer texture format not supported");
					break;
				}
			}
		}

		if (m_DepthAttachmentSpecs.TextureFormat != FrameBufferTextureFormat::None)
		{
			Utils::CreateTextures(multiSamples, &m_DepthAttachment, 1);
			Utils::BindTextures(multiSamples, m_DepthAttachment);

			switch (m_DepthAttachmentSpecs.TextureFormat)
			{
			case FrameBufferTextureFormat::DEPTH24STENCIL8:
				Utils::AttachDepthTexture(m_DepthAttachment, m_Specs.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT,
					m_Specs.Width, m_Specs.Height);
				break;
			default:
				DBT_CORE_ASSERT(false, "Frame buffer texture format not supported");
				break;
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			DBT_CORE_ASSERT(m_ColorAttachments.size() <= 4);
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, };
			glDrawBuffers(m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			// Depth-pass only
			glDrawBuffer(GL_NONE);
		}

		GLCall(DBT_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Frame buffer is incomplete"));		
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	void OpenGLFrameBuffer::Resize(uint32_t x, uint32_t y)
	{
		if (x == 0 || y == 0 || x > MAX_FRAME_BUFFER_SIZE || y > MAX_FRAME_BUFFER_SIZE)	
		{
			Log.CoreWarn("Attempted to resize frame buffer to incorrect size: {0}, {1}", x, y);
			return;
		}

		m_Specs.Width = x;
		m_Specs.Height = y;
		Invalidate();
	}

	int OpenGLFrameBuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		//Bind();

		int pixelData;

		DBT_ASSERT(attachmentIndex < m_ColorAttachments.size() && attachmentIndex >= 0);
		GLCall(glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex));
		GLCall(glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData));

		//Unbind();

		return pixelData;
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGLFrameBuffer::Bind() const
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));
		GLCall(glViewport(0, 0, m_Specs.Width, m_Specs.Height));
	}

	void OpenGLFrameBuffer::Unbind() const
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}
}