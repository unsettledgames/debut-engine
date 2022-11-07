#pragma once

#include <Debut/Rendering/Renderer/RendererAPI.h>

namespace Debut
{
	enum class CullingMode { CullFront = 0, CullBack };

	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetClearColor(const glm::vec4& color) 
		{ 
			s_RendererAPI->SetClearColor(color); 
		}

		inline static void CullFront()
		{
			s_RendererAPI->CullFront();
		}

		inline static void CullBack()
		{
			s_RendererAPI->CullBack();
		}

		inline static void Clear() 
		{ 
			s_RendererAPI->Clear(); 
		}

		inline static void ClearDepth()
		{
			s_RendererAPI->ClearDepth();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& va, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(va, indexCount);
		}

		inline static void DrawLines(const Ref<VertexArray>& va, uint32_t vertexCount = 0)
		{
			s_RendererAPI->DrawLines(va, vertexCount);
		}

		inline static void DrawPoints(const Ref<VertexArray>& va, uint32_t vertexCount = 0)
		{
			s_RendererAPI->DrawPoints(va, vertexCount);
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		inline static void SetLineWidth(float thickness)
		{
			s_RendererAPI->SetLineWidth(thickness);
		}

		inline static void SetPointSize(float thickness)
		{
			s_RendererAPI->SetPointSize(thickness);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};
}