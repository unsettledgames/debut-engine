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

	class RenderTexture
	{
	public:
		static Ref<RenderTexture> Create(float width, float height, Ref<FrameBuffer> buffer);
		~RenderTexture() = default;

		virtual void Draw(Ref<Shader> shader) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		inline uint32_t GetRendererID() { return m_RendererID; }
		inline float GetWidth() { return m_Width; }
		inline float GetHeight() { return m_Height; }

	protected:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<VertexArray> m_VertexArray;

		uint32_t m_RendererID;

		float m_Width;
		float m_Height;
	};
}