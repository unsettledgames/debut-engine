#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Rendering/Structures/FrameBuffer.h>


namespace Debut
{
	class FrameBuffer;
	class VertexBuffer;
	class IndexBuffer;
	class VertexArray;

	class Shader;
	class PostProcessingStack;

	enum class RenderTextureMode {Color = 0, Depth};

	class RenderTexture
	{
	public:
		static Ref<RenderTexture> Create(float width, float height, Ref<FrameBuffer> buffer, RenderTextureMode mode);
		~RenderTexture() = default;

		virtual void Draw(Ref<Shader> shader, Ref< PostProcessingStack> postProcessingStack = nullptr) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		inline void SetFrameBuffer(Ref<FrameBuffer> buffer) { m_FrameBuffer = buffer; }

		inline uint32_t GetRendererID() { return m_RendererID; }
		inline float GetWidth() { return m_Width; }
		inline float GetHeight() { return m_Height; }

	protected:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<VertexArray> m_VertexArray;
		Ref<FrameBuffer> m_FrameBuffer;

		uint32_t m_RendererID;

		float m_Width;
		float m_Height;
		RenderTextureMode m_Mode;
	};
}