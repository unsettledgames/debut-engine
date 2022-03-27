#pragma once

#include "RendererAPI.h"

namespace Debut
{
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

		inline static void Clear() 
		{ 
			s_RendererAPI->Clear(); 
		}

		inline static void DrawIndexed(const Ref<VertexArray>& va, uint32_t indexCount = 0)
		{ 
			s_RendererAPI->DrawIndexed(va, indexCount); 
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};
}