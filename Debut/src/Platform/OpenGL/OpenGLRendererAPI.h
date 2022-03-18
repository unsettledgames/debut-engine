#pragma once
#include "Debut/Renderer/RendererAPI.h"

namespace Debut
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;

		virtual void Clear() override;
		virtual void SetClearColor(const glm::vec4 color) override;

		virtual void DrawIndexed(const Ref<VertexArray>& va) override;
	private:
	};
}