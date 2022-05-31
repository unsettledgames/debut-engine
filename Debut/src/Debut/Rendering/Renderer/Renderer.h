#pragma once

#include "RendererAPI.h"

namespace Debut
{
	class Renderer
	{
	public:
		static void Init();

		static void OnWindowResized(uint32_t width, uint32_t height);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
		inline static void SetApi(RendererAPI::API val) { RendererAPI::SetAPI(val); }
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};
}