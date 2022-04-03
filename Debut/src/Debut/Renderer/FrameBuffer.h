#pragma once

namespace Debut
{
	struct FrameBufferSpecs
	{
		uint32_t Width, Height;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;
		static Ref<FrameBuffer> Create(const FrameBufferSpecs& specs);

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void Invalidate() = 0;
		virtual void Resize(uint32_t x, uint32_t y) = 0;

		virtual const FrameBufferSpecs& GetSpecs() = 0;
		virtual uint32_t GetColorAttachment() const = 0;
		virtual uint32_t GetDepthAttachment() const = 0;
	private:
	};
}
