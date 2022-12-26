#pragma once

#include <Debut/Core/Core.h>
#include <Debut/Rendering/Structures/FrameBuffer.h>
#include <Debut/Rendering/Shader.h>

#include <unordered_map>

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
		RenderTexture(FrameBufferSpecifications bufferSpecs);
		~RenderTexture() = default;

		void Draw(Ref<Shader> shader, std::unordered_map<std::string, 
			ShaderUniform>& properties = std::unordered_map<std::string, ShaderUniform>());
		void Draw(Ref<FrameBuffer> startBuffer, Ref<Shader> startShader, Ref<PostProcessingStack> postProcessing);
		void DrawOverlay(Ref<FrameBuffer> bottomBuffer, Ref<FrameBuffer> topBuffer, Ref<Shader> shader);
		void Resize(uint32_t x, uint32_t y);

		virtual void BindTexture() = 0;
		virtual void UnbindTexture() = 0;

		inline Ref<FrameBuffer> GetTopFrameBuffer() { return m_PrevBuffer; }
		inline void SetSourceBuffer(Ref<FrameBuffer> buffer) { m_FrameBuffer = buffer; }

		inline Ref<FrameBuffer> GetFrameBuffer() { return m_FrameBuffer; }
		inline uint32_t GetRendererID() { return m_RendererID; }

	protected:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<VertexArray> m_VertexArray;
		Ref<FrameBuffer> m_FrameBuffer;

		uint32_t m_RendererID;

		RenderTextureMode m_Mode;
		Ref<Shader> m_BasicFullScreen;
		Ref<RenderTexture> m_Target;

		Ref<FrameBuffer> m_PrevBuffer;
		Ref<FrameBuffer> m_NextBuffer;
		std::unordered_map<uint32_t, std::vector<Ref<FrameBuffer>>> m_DownscaledBuffers;
	};
}