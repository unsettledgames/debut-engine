#pragma once
#include "Debut/Renderer/FrameBuffer.h"

namespace Debut
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecs& specs);
		virtual ~OpenGLFrameBuffer();

		void Invalidate();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual uint32_t GetColorAttachment() const { return m_ColorAttachment; }
		inline virtual uint32_t GetDepthAttachment() const { return m_DepthAttachment; }
		inline virtual FrameBufferSpecs& GetSpecs() override { return m_Specs; }

	private:
		uint32_t m_RendererID;
		uint32_t m_ColorAttachment;
		uint32_t m_DepthAttachment;

		FrameBufferSpecs m_Specs;
	};
}
