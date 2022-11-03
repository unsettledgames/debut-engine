#include <Debut/Rendering/Structures/ShadowMap.h>
#include <Debut/Rendering/Structures/FrameBuffer.h>

namespace Debut
{
	ShadowMap::ShadowMap(uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
	{
		FrameBufferSpecifications specs;
		specs.Attachments = { FrameBufferTextureFormat::Depth };
		specs.Width = m_Width;
		specs.Height = m_Height;

		m_FrameBuffer = FrameBuffer::Create(specs);
	}

	void ShadowMap::Bind()
	{
		m_FrameBuffer->Bind();
	}

	void ShadowMap::BindAsTexture(uint32_t slot)
	{
		m_FrameBuffer->BindAsTexture(slot);
	}

	void ShadowMap::Unbind()
	{
		m_FrameBuffer->Unbind();
	}

	uint32_t ShadowMap::GetRendererID()
	{
		return m_FrameBuffer->GetDepthAttachment();
	}
}