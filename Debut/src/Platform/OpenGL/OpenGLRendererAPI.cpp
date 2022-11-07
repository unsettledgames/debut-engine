#include "Debut/dbtpch.h"
#include "OpenGLRendererAPI.h"
#include "OpenGLError.h"
#include <Debut/Rendering/Structures/Buffer.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <glad/glad.h>

namespace Debut
{
	void OpenGLRendererAPI::Init()
	{
		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glEnable(GL_LINE_SMOOTH));

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		GLCall(glEnable(GL_CULL_FACE));
		GLCall(glCullFace(GL_BACK));
		GLCall(glFrontFace(GL_CCW));
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::ClearDepth()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4 color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::CullFront()
	{
		GLCall(glCullFace(GL_FRONT));
	}

	void OpenGLRendererAPI::CullBack()
	{
		GLCall(glCullFace(GL_BACK));
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& va, uint32_t indexCount)
	{
		uint64_t count = indexCount == 0 ? va->GetIndexBuffer()->GetCount() : indexCount;
		va->Bind();
		GLCall(glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr));
		va->Unbind();
	}

	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& va, uint32_t vertexCount)
	{
		va->Bind();
		GLCall(glDrawArrays(GL_LINES, 0, vertexCount));
		va->Unbind();
	}

	void OpenGLRendererAPI::DrawPoints(const Ref<VertexArray>& va, uint32_t vertexCount)
	{
		va->Bind();
		GLCall(glDrawArrays(GL_POINTS, 0, vertexCount));
		va->Unbind();
	}


	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		GLCall(glViewport(x, y, width, height));
	}

	void OpenGLRendererAPI::SetLineWidth(float thickness)
	{
		GLCall(glLineWidth(thickness));
	}

	void OpenGLRendererAPI::SetPointSize(float thickness)
	{
		GLCall(glPointSize(thickness));
	}
}