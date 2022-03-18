#pragma once

#include "Debut/Renderer/VertexArray.h"
#include <glm/glm.hpp>

namespace Debut
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};

	public:
		virtual void Clear() = 0;
		virtual void SetClearColor(const glm::vec4 color) = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& va) = 0;

		inline static API GetAPI() { return s_API; }
		inline static void SetAPI(API val) { s_API = val; }

	private:
		static API s_API;
	};
}

