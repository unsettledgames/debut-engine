#pragma once
#include "Debut/Renderer/RendererAPI.h"

namespace Debut
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Clear() override;
		virtual void SetClearColor(const glm::vec4 color) override;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& va) override;
	private:
	};
}