#pragma once

#include "RendererAPI.h"

namespace Debut
{
	class RenderCommand
	{
	public:
		inline static void SetClearColor(const glm::vec4& color) 
		{ 
			s_RendererAPI->SetClearColor(color); 
		}

		inline static void Clear() 
		{ 
			s_RendererAPI->Clear(); 
		}

		static void DrawIndexed(const Ref<VertexArray>& va) 
		{ 
			s_RendererAPI->DrawIndexed(va); 
		}

	private:
		static RendererAPI* s_RendererAPI;
	};
}