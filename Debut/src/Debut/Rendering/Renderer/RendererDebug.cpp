#include <Debut/Rendering/Renderer/RendererDebug.h>
#include <Debut/Rendering/Renderer/RenderCommand.h>

#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>

#include <Debut/Rendering/Camera.h>

namespace Debut
{
	RendererDebugStorage RendererDebug::s_Storage;

	void RendererDebug::Init()
	{
		RenderCommand::SetLineWidth(1.3f);
		RenderCommand::SetPointSize(3.0f);

		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position", false},
			{ShaderDataType::Float4, "a_Color", false }
		};

		s_Storage.LineVertexArray = VertexArray::Create();
		s_Storage.LineVertexBuffer = VertexBuffer::Create((uint32_t)0, s_Storage.MaxLines * sizeof(LineVertex));
		s_Storage.LineVertexBuffer->SetLayout(layout);
		s_Storage.LineVertexBase = new LineVertex[s_Storage.MaxLines];
		s_Storage.CurrentLineVertex = s_Storage.LineVertexBase;

		s_Storage.PointVertexArray = VertexArray::Create();
		s_Storage.PointVertexBuffer = VertexBuffer::Create((uint32_t)0, s_Storage.MaxPoints * sizeof(PointVertex));
		s_Storage.PointVertexBuffer->SetLayout(layout);
		s_Storage.PointVertexBase = new PointVertex[s_Storage.MaxPoints];
		s_Storage.CurrentPointVertex = s_Storage.PointVertexBase;

		s_Storage.LineShader = Shader::Create("assets\\shaders\\line.glsl");
		s_Storage.PointShader = Shader::Create("assets\\shaders\\point.glsl");
	}

	void RendererDebug::Shutdown()
	{
		delete[] s_Storage.LineVertexBase;
		delete[] s_Storage.PointVertexBase;
	}

	void RendererDebug::BeginScene(Camera& camera, glm::mat4& transform)
	{
		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);

		s_Storage.LineShader->Bind();
		s_Storage.LineShader->SetMat4("u_ViewProjection", viewProj);
		s_Storage.LineShader->SetMat4("u_ViewMatrix", glm::inverse(transform));
		s_Storage.LineShader->SetMat4("u_ProjectionMatrix", camera.GetProjection());
		s_Storage.LineShader->Unbind();

		s_Storage.PointShader->Bind();
		s_Storage.PointShader->SetMat4("u_ViewProjection", viewProj);
		s_Storage.PointShader->SetMat4("u_ViewMatrix", glm::inverse(transform));
		s_Storage.PointShader->SetMat4("u_ProjectionMatrix", camera.GetProjection());
		s_Storage.PointShader->Unbind();

		StartLineBatch();
		StartPointBatch();
	}

	void RendererDebug::EndScene()
	{
		if (s_Storage.LineCount)
		{
			uint64_t dataSize = (uint8_t*)s_Storage.CurrentLineVertex - (uint8_t*)s_Storage.LineVertexBase;
			s_Storage.LineVertexBuffer->SetData(s_Storage.LineVertexBase, dataSize);

			s_Storage.LineShader->Bind();
			FlushLines();
			s_Storage.LineShader->Unbind();
		}

		if (s_Storage.PointCount)
		{
			uint64_t dataSize = (uint8_t*)s_Storage.CurrentPointVertex - (uint8_t*)s_Storage.PointVertexBase;
			s_Storage.PointVertexBuffer->SetData(s_Storage.PointVertexBase, dataSize);

			s_Storage.PointShader->Bind();
			FlushPoints();
			s_Storage.PointShader->Unbind();
		}
	}

	void RendererDebug::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		if (s_Storage.LineCount > s_Storage.MaxLines)
		{
			FlushLines();
			StartLineBatch();
		}

		s_Storage.CurrentLineVertex->Position = p0;
		s_Storage.CurrentLineVertex->Color = color;
		s_Storage.CurrentLineVertex++;

		s_Storage.CurrentLineVertex->Position = p1;
		s_Storage.CurrentLineVertex->Color = color;
		s_Storage.CurrentLineVertex++;

		s_Storage.LineCount++;
	}

	void RendererDebug::DrawPoint(const glm::vec3& p0, const glm::vec4& color)
	{
		if (s_Storage.PointCount > s_Storage.MaxPoints)
		{
			FlushPoints();
			StartPointBatch();
		}

		s_Storage.CurrentPointVertex->Position = p0;
		s_Storage.CurrentPointVertex->Color = color;
		s_Storage.CurrentPointVertex++;

		s_Storage.PointCount++;
	}

	void RendererDebug::DrawRect(const glm::mat4& transform, const glm::vec2& size, const glm::vec2& offset, const glm::vec4& color)
	{
		glm::vec3 topRight, bottomRight, topLeft, bottomLeft;

		topRight = transform * glm::vec4(-size.x / 2 + offset.x, size.y / 2 + offset.y, 0.0f, 1.0f);
		bottomRight = transform * glm::vec4(-size.x / 2 + offset.x, -size.y / 2 + offset.y, 0.0f, 1.0f);
		topLeft = transform * glm::vec4(size.x / 2 + offset.x, size.y / 2 + offset.y, 0.0f, 1.0f);
		bottomLeft = transform * glm::vec4(size.x / 2 + offset.x, -size.y / 2 + offset.y, 0.0f, 1.0f);

		DrawLine(topRight, bottomRight, color);
		DrawLine(topLeft, bottomLeft, color);
		DrawLine(topLeft, topRight, color);
		DrawLine(bottomLeft, bottomRight, color);
	}

	void RendererDebug::FlushLines()
	{
		// Draw call
		RenderCommand::DrawLines(s_Storage.LineVertexArray, s_Storage.LineCount);
		s_Storage.LineCount = 0;
		s_Storage.CurrentLineVertex = s_Storage.LineVertexBase;
	}

	void RendererDebug::FlushPoints()
	{
		// Draw call
		RenderCommand::DrawPoints(s_Storage.PointVertexArray, s_Storage.PointCount);
		s_Storage.PointCount = 0;
		s_Storage.CurrentPointVertex = s_Storage.PointVertexBase;
	}

	void RendererDebug::StartLineBatch()
	{
		s_Storage.LineCount = 0;
		s_Storage.CurrentLineVertex = s_Storage.LineVertexBase;
	}

	void RendererDebug::StartPointBatch()
	{
		s_Storage.PointCount = 0;
		s_Storage.CurrentPointVertex = s_Storage.PointVertexBase;
	}
}