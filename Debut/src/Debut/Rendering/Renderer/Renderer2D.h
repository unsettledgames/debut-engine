#pragma once

#include <glm/glm.hpp>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/Texture.h>
#include <Debut/Rendering/SubTexture2D.h>
#include <Debut/Rendering/Camera.h>
#include <array>
#include <Debut/Scene/Components.h>

namespace Debut
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		glm::vec4 Color;
		float TexIndex;
		float TilingFactor;

		// Editor only
		int EntityID = -1;
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;

		// Unity-like line rendering? In that case Quad + proper shader!
	};

	struct Render2DStats
	{
		uint32_t DrawCalls;
		uint32_t QuadCount;

		uint32_t GetTotalVertexCount() { return QuadCount * 4; }
		uint32_t GetIndexCount() { return QuadCount * 6; }
	};

	struct Renderer2DStorage
	{
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		// TODO: RenderCapabilities
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture> WhiteTexture;

		Ref<Debut::VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		Ref<Shader> LineShader;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		uint32_t LineVertexCount = 0;
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;

		std::array<Ref<Texture>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		glm::vec4 QuadVertexPositions[4];
		Render2DStats Stats;
	};

	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4 transform);
		static void EndScene();
		static void Flush();

		// Primitives
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotationAngle, const glm::vec4 color);
		static void DrawQuad(const glm::mat4& transform, const glm::vec4 color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotationAngle, const Ref<Texture>& texture, float tilingFactor = 1);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotationAngle, const Ref<SubTexture2D>& texture, float tilingFactor = 1);

		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
		static void DrawRect(const glm::mat4& transform, const glm::vec2& size, const glm::vec2& offset, const glm::vec4& color);

		static void DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& src, int entityID);
		static void ResetStats();
		static Render2DStats GetStats() { return s_Data.Stats; }
	
	private:
		static void FlushAndReset();
		static void StartBatch();

	private:
		static Renderer2DStorage s_Data;
		

	};
}
