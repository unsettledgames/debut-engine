#pragma once
#include "Debut/Renderer/FrameBuffer.h"

namespace Debut
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecs& specs);
		virtual ~OpenGLFrameBuffer();

		virtual void Invalidate() override;
		virtual void Resize(uint32_t x, uint32_t y) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual uint32_t GetColorAttachment() const { return m_ColorAttachment; }
		inline virtual uint32_t GetDepthAttachment() const { return m_DepthAttachment; }
		inline virtual FrameBufferSpecs& GetSpecs() override { return m_Specs; }

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_DepthAttachment = 0;
		uint32_t m_ColorAttachment = 0;

		FrameBufferSpecs m_Specs;
	};
}
