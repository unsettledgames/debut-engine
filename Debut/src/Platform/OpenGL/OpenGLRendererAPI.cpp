#include "Debut/dbtpch.h"
#include "OpenGLRendererAPI.h"
#include "OpenGLError.h"
#include <glad/glad.h>

namespace Debut
{
	void OpenGLRendererAPI::Init()
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4 color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& va, uint32_t indexCount)
	{
		uint32_t count = indexCount == 0 ? va->GetIndexBuffer()->GetCount() : indexCount;
		GLCall(glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr));
		// Reset the texture after a draw call
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}
}