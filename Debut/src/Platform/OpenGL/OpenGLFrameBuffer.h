#pragma once
#include "Debut/Renderer/FrameBuffer.h"

namespace Debut
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecifications& specs);
		virtual ~OpenGLFrameBuffer();

		virtual void Invalidate() override;
		virtual void Resize(uint32_t x, uint32_t y) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;
		virtual void ClearAttachment(uint32_t index, int value) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		inline virtual uint32_t GetColorAttachment(int idx = 0) const override { DBT_ASSERT(idx < m_ColorAttachments.size()); return m_ColorAttachments[idx]; }
		inline virtual uint32_t GetDepthAttachment() const override { return m_DepthAttachment; }
		inline virtual FrameBufferSpecifications& GetSpecs() override { return m_Specs; }

	private:
		uint32_t m_RendererID = 0;

		FrameBufferSpecifications m_Specs;

		// Renderer IDs for attachments
		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment;

		// Attachment formats
		std::vector<FrameBufferTextureSpecification> m_ColorAttachmentSpecs;
		FrameBufferTextureSpecification m_DepthAttachmentSpecs = FrameBufferTextureFormat::None;
	};
}
