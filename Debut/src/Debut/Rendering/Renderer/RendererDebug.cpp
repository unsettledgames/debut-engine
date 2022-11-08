#include <Debut/Rendering/Renderer/RendererDebug.h>
#include <Debut/Rendering/Renderer/RenderCommand.h>

#include <Debut/Scene/Components.h>

#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>

#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Camera.h>

namespace Debut
{
	RendererDebugStorage RendererDebug::s_Storage;

	void RendererDebug::Init()
	{
		RenderCommand::SetLineWidth(0.05f);
		RenderCommand::SetPointSize(12.0f);

		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position", false},
			{ShaderDataType::Float4, "a_Color", false }
		};

		s_Storage.LineVertexArray = VertexArray::Create();
		s_Storage.LineVertexBuffer = VertexBuffer::Create((uint32_t)0, s_Storage.MaxLines * sizeof(LineVertex));
		s_Storage.LineVertexBuffer->SetLayout(layout);
		s_Storage.LineVertexBase = new LineVertex[s_Storage.MaxLines];
		s_Storage.LineVertexArray->AddVertexBuffer(s_Storage.LineVertexBuffer);
		s_Storage.CurrentLineVertex = s_Storage.LineVertexBase;

		s_Storage.PointVertexArray = VertexArray::Create();
		s_Storage.PointVertexBuffer = VertexBuffer::Create((uint32_t)0, s_Storage.MaxPoints * sizeof(PointVertex));
		s_Storage.PointVertexBuffer->SetLayout(layout);
		s_Storage.PointVertexBase = new PointVertex[s_Storage.MaxPoints];
		s_Storage.PointVertexArray->AddVertexBuffer(s_Storage.PointVertexBuffer);
		s_Storage.CurrentPointVertex = s_Storage.PointVertexBase;

		s_Storage.LineShader = Shader::Create("assets\\shaders\\line.glsl");
		s_Storage.PointShader = Shader::Create("assets\\shaders\\point.glsl");
	}

	void RendererDebug::Shutdown()
	{
		delete[] s_Storage.LineVertexBase;
		delete[] s_Storage.PointVertexBase;
	}

	void RendererDebug::BeginScene(Camera& camera, const glm::mat4& transform)
	{
		RenderCommand::DisableCulling();
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

		RenderCommand::EnableCulling();
	}

	void RendererDebug::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, bool highlightVertices)
	{
		if (s_Storage.LineCount > s_Storage.MaxLines)
		{
			FlushLines();
			StartLineBatch();
		}

		if (highlightVertices)
		{
			DrawPoint(p0, color);
			DrawPoint(p1, color);
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

	void RendererDebug::DrawRect(const glm::mat4& transform, const glm::vec2& size, const glm::vec2& offset, const glm::vec4& color, bool highlightVertices)
	{
		glm::vec3 topRight, bottomRight, topLeft, bottomLeft;

		topRight = transform * glm::vec4(-size.x / 2 + offset.x, size.y / 2 + offset.y, 0.0f, 1.0f);
		bottomRight = transform * glm::vec4(-size.x / 2 + offset.x, -size.y / 2 + offset.y, 0.0f, 1.0f);
		topLeft = transform * glm::vec4(size.x / 2 + offset.x, size.y / 2 + offset.y, 0.0f, 1.0f);
		bottomLeft = transform * glm::vec4(size.x / 2 + offset.x, -size.y / 2 + offset.y, 0.0f, 1.0f);

		if (highlightVertices)
		{
			DrawPoint(topRight, color);
			DrawPoint(bottomRight, color);
			DrawPoint(topLeft, color);
			DrawPoint(bottomLeft, color);
		}

		DrawLine(topRight, bottomRight, color);
		DrawLine(topLeft, bottomLeft, color);
		DrawLine(topLeft, topRight, color);
		DrawLine(bottomLeft, bottomRight, color);
	}

	void RendererDebug::DrawCircle(float radius, const glm::vec3 center, glm::mat4& transform, float iterations)
	{
		float angleIncrease = glm::radians(360.0f) / iterations;
		float currentAngle = 0;
		glm::vec3 scale = glm::vec3(glm::compMin(glm::vec3(transform[0][0], transform[1][1], transform[2][2])));

		// Use lines to approximate a circle
		for (uint32_t i = 0; i < iterations; i++)
		{
			RendererDebug::DrawLine(
				glm::vec3(transform * glm::vec4(center + radius * glm::vec3(glm::cos(currentAngle), 
					glm::sin(currentAngle), 0.0f), 1.0f)),
				glm::vec3(transform * glm::vec4(center + radius * glm::vec3(glm::cos(currentAngle + angleIncrease),
					glm::sin(currentAngle + angleIncrease), 0.0f), 1.0f)),
				glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			currentAngle += angleIncrease;
		}
	}

	void RendererDebug::DrawSphere(float radius, const glm::vec3& center, const glm::vec3& rot,
		const glm::vec3& scale, const glm::mat4 cameraView, const glm::mat4& transform)
	{
		float nIterations = 40;
		float angleIncrease = glm::radians(360.0f) / nIterations;
		float currentAngle = 0;
		glm::mat4 inverseView = glm::inverse(cameraView);
		glm::vec3 transformedCenter = transform * glm::vec4(center, 1.0f);

		// Create camera space
		glm::vec3 normal = glm::normalize(glm::vec3(inverseView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
		glm::vec3 tangent = glm::normalize(glm::vec3(inverseView * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));
		glm::vec3 bitangent = glm::normalize(glm::vec3(inverseView * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
		glm::mat4 cameraTangent = glm::mat4(glm::vec4(bitangent, 0.0f), glm::vec4(tangent, 0.0f), glm::vec4(normal, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		cameraTangent = glm::mat4(glm::mat3(cameraTangent));
		// Use lines to approximate a circle
		for (uint32_t i = 0; i < nIterations; i++)
		{
			RendererDebug::DrawLine(
				transformedCenter + glm::vec3(glm::mat4(cameraTangent) * (glm::vec4(radius * glm::compMin(scale) *
					glm::vec3(glm::cos(currentAngle), glm::sin(currentAngle), 0.0f), 1.0f))),
				transformedCenter + glm::vec3(glm::mat4(cameraTangent) * (glm::vec4(radius * glm::compMin(scale) *
					glm::vec3(glm::cos(currentAngle + angleIncrease), glm::sin(currentAngle + angleIncrease), 0.0f), 1.0f))),
				glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			currentAngle += angleIncrease;
		}

		// Draw the circles
		RendererDebug::DrawCircle(radius, glm::vec3(0.0f), transform * glm::rotate(glm::translate(
			glm::mat4(1.0f), center), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)), 40);
		RendererDebug::DrawCircle(radius, glm::vec3(0.0f), transform * glm::rotate(glm::translate(
			glm::mat4(1.0f), center), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), 40);
		RendererDebug::DrawCircle(radius, glm::vec3(0.0f), transform * glm::rotate(glm::translate(
			glm::mat4(1.0f), center), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)), 40);

	}

	void RendererDebug::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		glm::vec4 points[4] = { {-0.5f, -0.5f, 0.0f, 1.0f}, {0.5f, -0.5f, 0.0f, 1.0f}, 
			{0.5f, 0.5f, 0.0f, 1.0f}, {-0.5f, 0.5f, 0.0f, 1.0f} };

		glm::vec4 transformedPoints[4];
		for (uint32_t i = 0; i < 4; i++)
			transformedPoints[i] = transform * points[i];

		// 4 lines and the diagonal
		for (uint32_t i=0; i<4; i++)
			DrawLine(transformedPoints[i], transformedPoints[(i + 1) % 4], color);
		DrawLine(transformedPoints[0], transformedPoints[2], color);
	}

	void RendererDebug::DrawBox(const glm::vec3& size, const glm::vec3& offset, const glm::mat4& transform, const glm::vec4& color)
	{
		glm::vec3 hSize = size / 2.0f;
		std::vector<glm::vec3> points = {
			glm::vec3(hSize.x, hSize.y, hSize.z) + offset,
			glm::vec3(-hSize.x, hSize.y, hSize.z) + offset,
			glm::vec3(hSize.x,-hSize.y, hSize.z) + offset,
			glm::vec3(-hSize.x,-hSize.y, hSize.z) + offset,
			glm::vec3(hSize.x, hSize.y,-hSize.z) + offset,
			glm::vec3(-hSize.x, hSize.y,-hSize.z) + offset,
			glm::vec3(hSize.x,-hSize.y,-hSize.z) + offset,
			glm::vec3(-hSize.x,-hSize.y,-hSize.z) + offset
		};
		std::vector<glm::vec3> transformedPoints;
		transformedPoints.resize(8);

		for (uint32_t i = 0; i < points.size(); i++)
			transformedPoints[i] = glm::vec3(transform * glm::vec4(points[i], 1.0f));

		// Render lines
		for (uint32_t i = 0; i < points.size(); i++)
		{
			RendererDebug::DrawPoint(transformedPoints[i], glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			for (uint32_t j = i; j < points.size(); j++)
			{
				uint32_t same = 0;
				for (uint32_t k = 0; k < 3; k++)
					if (points[i][k] == points[j][k])
						same++;

				if (same == 2)
					RendererDebug::DrawLine(transformedPoints[i], transformedPoints[j], glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
			}
		}
	}

	void RendererDebug::DrawPolygon(const std::vector<std::vector<glm::vec2>>& triangles,
		const glm::vec3& offset, const glm::mat4& transform, const glm::vec4& color)
	{
		// Render triangle lines
		for (uint32_t i = 0; i < triangles.size(); i++)
		{
			for (uint32_t j = 0; j < 3; j++)
			{
				glm::vec2 curr = triangles[i][j];
				glm::vec2 next = triangles[i][(j + 1) % 3];
				RendererDebug::DrawLine(
					glm::vec3(transform * glm::vec4(offset + glm::vec3(curr, 0.0f), 1.0f)),
					glm::vec3(transform * glm::vec4(offset + glm::vec3(next, 0.0f), 1.0f)),
					glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
				);
			}
		}
	}
	

	void RendererDebug::DrawMesh(UUID& mesh, const glm::vec3& offset, const glm::mat4& transform, const glm::vec4& color)
	{
		Ref<Mesh> meshAsset = AssetManager::Request<Mesh>(mesh);
		if (meshAsset == nullptr)
			return;
		std::vector<float>& vertices = meshAsset->GetPositions();
		std::vector<int>& indices = meshAsset->GetIndices();

		std::vector<glm::vec3> transformedVertices;
		glm::vec3 transformedOffset = glm::mat4(glm::mat3(transform)) * glm::vec4(offset, 1.0f);
		transformedVertices.resize(vertices.size() / 3);

		for (uint32_t i = 0; i < vertices.size(); i+=3)
			transformedVertices[i / 3] = transform * glm::vec4(vertices[i], vertices[i + 1], vertices[i + 2], 1.0f);

		for (uint32_t i = 0; i < indices.size(); i += 3)
			for (uint32_t j = 0; j < 3; j++)
				DrawLine(transformedVertices[indices[i + j]] + transformedOffset, 
					transformedVertices[indices[i + (j + 1)%3]] + transformedOffset, color, false);
	}
	
	void RendererDebug::FlushLines()
	{
		// Render on top 
		RenderCommand::ClearDepth();
		// Draw call
		RenderCommand::DrawLines(s_Storage.LineVertexArray, s_Storage.LineCount * 2);
		s_Storage.LineCount = 0;
		s_Storage.CurrentLineVertex = s_Storage.LineVertexBase;
	}

	void RendererDebug::FlushPoints()
	{
		// Render on top of everything, including lines
		RenderCommand::ClearDepth();
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