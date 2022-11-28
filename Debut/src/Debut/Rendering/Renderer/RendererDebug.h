#pragma once

#include <Debut/Core/Core.h>
#include <glm/glm.hpp>

#include <vector>

namespace Debut
{
	class VertexArray;
	class IndexBuffer;
	class VertexBuffer;

	class Shader;
	class UUID;
	class SceneCamera;

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct PointVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct RendererDebugStorage
	{
		static const int MaxLines = 2000000;
		static const int MaxPoints = 4000000;

		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;

		Ref<VertexArray> PointVertexArray;
		Ref<VertexBuffer> PointVertexBuffer;

		Ref<Shader> PointShader;
		Ref<Shader> LineShader;

		uint32_t LineCount = 0;
		LineVertex* LineVertexBase = nullptr;
		LineVertex* CurrentLineVertex = nullptr;

		uint32_t PointCount = 0;
		PointVertex* PointVertexBase = nullptr;
		PointVertex* CurrentPointVertex = nullptr;
	};

	struct TransformComponent;

	class RendererDebug
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(SceneCamera& camera);
		static void EndScene();
		
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, bool highlightVertices = false);
		static void DrawPoint(const glm::vec3& p0, const glm::vec4& color);
		static void DrawRect(const glm::mat4& transform, const glm::vec2& size, const glm::vec2& offset, const glm::vec4& color, bool highlightVertices = false);
		static void DrawCircle(float radius, const glm::vec3 center,  glm::mat4& transform, float iterations);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
		static void DrawPolygon(const std::vector<std::vector<glm::vec2>>& triangles,
								const glm::vec3& offset, const glm::mat4& transform, const glm::vec4& color);
		static void DrawBox(const glm::vec3& size, const glm::vec3& offset, const glm::mat4& transform, const glm::vec4& color);
		static void DrawMesh(UUID& mesh, const glm::vec3& offset, const glm::mat4& transform, const glm::vec4& color);
		static void DrawSphere(float radius, const glm::vec3& center, const glm::vec3& rot,
			const glm::vec3& scale, const glm::mat4 cameraView, const glm::mat4& transform);

	private:
		static void FlushLines();
		static void FlushPoints();

		static void StartLineBatch();
		static void StartPointBatch();

	private:
		static RendererDebugStorage s_Storage;
	};
}
