#pragma once

namespace Debut
{
	enum class FrameBufferTextureFormat
	{
		None,

		// Color
		RGBA8,
		RED_INTEGER,

		// Depth / stencil
		DEPTH24STENCIL8,

		// Default values
		Depth = DEPTH24STENCIL8,
		Color = RGBA8
	};

	struct FrameBufferTextureSpecification
	{
		FrameBufferTextureSpecification() = default;
		FrameBufferTextureSpecification(FrameBufferTextureFormat format) : TextureFormat(format) {}

		FrameBufferTextureFormat TextureFormat = FrameBufferTextureFormat::None;
		// todo: filtering / wrap mode
	};

	struct FrameBufferAttachmentSpecification
	{
		FrameBufferAttachmentSpecification() = default;
		FrameBufferAttachmentSpecification(const std::initializer_list<FrameBufferTextureSpecification>& attachments) :
			Attachments(attachments) {}

		std::vector<FrameBufferTextureSpecification> Attachments;
	};

	struct FrameBufferSpecifications
	{
		uint32_t Width = 1920, Height = 1080;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
		FrameBufferAttachmentSpecification Attachments;
	};

	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;
		static Ref<FrameBuffer> Create(const FrameBufferSpecifications& specs);

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void Invalidate() = 0;
		virtual void Resize(uint32_t x, uint32_t y) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual const FrameBufferSpecifications& GetSpecs() = 0;
		virtual uint32_t GetColorAttachment(int idx = 0) const = 0;
		virtual uint32_t GetDepthAttachment() const = 0;
	private:
	};
}
