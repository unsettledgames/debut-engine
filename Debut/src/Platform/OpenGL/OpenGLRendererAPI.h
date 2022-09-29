#pragma once
#include "Debut/Rendering/Renderer/RendererAPI.h"

namespace Debut
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;

		virtual void Clear() override;
		virtual void SetClearColor(const glm::vec4 color) override;

		virtual void DrawIndexed(const Ref<VertexArray>& va, uint32_t indexCount = 0) override;
		virtual void DrawLines(const Ref<VertexArray>& va, uint32_t vertexCount) override;
		virtual void DrawPoints(const Ref<VertexArray>& va, uint32_t vertexCount) override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetLineWidth(float thickness) override;
		virtual void SetPointSize(float thickness) override;
	private:
	};
}