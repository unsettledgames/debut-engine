#pragma once
#include "Debut/Rendering/Structures/FrameBuffer.h"

namespace Debut
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecifications& specs);
		virtual ~OpenGLFrameBuffer();

		virtual void Invalidate() override;
		virtual void Resize(uint32_t x, uint32_t y) override;
		virtual void ClearAttachment(uint32_t index, int value) override;

		virtual int ReadRedPixel(uint32_t attachmentIndex, int x, int y) override;
		virtual int ReadDepthPixel(uint32_t index, int x, int y) override { return 0; }
		virtual glm::vec4 ReadPixel(uint32_t index, int x, int y) override;

		virtual void Bind() override;
		virtual void BindAsTexture(uint32_t slot) override;
		virtual void Unbind() override;

		inline virtual uint32_t GetColorAttachment(int idx = 0) const override { DBT_ASSERT(idx < m_ColorAttachments.size()); return m_ColorAttachments[idx]; }
		inline virtual uint32_t GetDepthAttachment() const override { return m_DepthAttachment; }
		inline virtual FrameBufferSpecifications& GetSpecs() override { return m_Specs; }

	private:
		uint32_t m_RendererID = 0;
		bool m_Bound = false;

		FrameBufferSpecifications m_Specs;

		// Renderer IDs for attachments
		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment;

		// Attachment formats
		std::vector<FrameBufferTextureSpecification> m_ColorAttachmentSpecs;
		FrameBufferTextureSpecification m_DepthAttachmentSpecs = FrameBufferTextureFormat::None;
	};
}
