#include "Debut/dbtpch.h"
#include "Debut/Renderer/RenderCommand.h"
#include "Debut/Renderer/VertexArray.h"
#include "Debut/Renderer/Shader.h"
#include "Renderer2D.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Debut
{
	Renderer2DStorage Renderer2D::s_Data;

	void Renderer2D::Init()
	{
		DBT_PROFILE_FUNCTION();

		s_Data.VertexArray = VertexArray::Create();

		// Initialize index buffer
		int* quadIndices = new int[s_Data.MaxIndices];
		uint32_t offset = 0;
		for (int i = 0; i < s_Data.MaxIndices; i+=6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		Ref<IndexBuffer> textIndBuffer = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		delete[] quadIndices;

		// Initialize vertex buffer
		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));
		s_Data.VertexArray = VertexArray::Create();
		BufferLayout squareLayout = {
			{ShaderDataType::Float3, "a_Position", false},
			{ShaderDataType::Float2, "a_UV", false},
			{ShaderDataType::Float4, "a_Color", false},
			{ShaderDataType::Float, "a_TexIndex", false},
			{ShaderDataType::Float, "a_TilingFactor", false}
		};
		s_Data.QuadVertexBuffer->SetLayout(squareLayout);

		// Setup vertex array
		s_Data.VertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);
		s_Data.VertexArray->AddIndexBuffer(textIndBuffer);
		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t data = 0xffffffff;
		s_Data.WhiteTexture->SetData(&data, sizeof(data));

		int samplers[s_Data.MaxTextureSlots];
		for (int i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		// Texture / shader setup
		s_Data.TextureShader = Shader::Create("C:/dev/Debut/Debut/assets/shaders/texture.glsl");
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		s_Data.QuadVertexPositions[0] = glm::vec4(-0.5, -0.5, 0, 1);
		s_Data.QuadVertexPositions[1] = glm::vec4(0.5, -0.5, 0, 1);
		s_Data.QuadVertexPositions[2] = glm::vec4(0.5, 0.5, 0, 1);
		s_Data.QuadVertexPositions[3] = glm::vec4(-0.5, 0.5, 0, 1);
	}

	void Renderer2D::Shutdown()
	{
		DBT_PROFILE_FUNCTION();
		delete[] s_Data.QuadVertexBufferBase;	
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		DBT_PROFILE_FUNCTION();
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_Data.TextureSlotIndex = 1;
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::EndScene()
	{
		DBT_PROFILE_FUNCTION();
		uint32_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::FlushAndReset()
	{
		EndScene();

		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::Flush()
	{
		// Bind textures
		for (int i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		// Draw call
		RenderCommand::DrawIndexed(s_Data.VertexArray, s_Data.QuadIndexCount);
		s_Data.Stats.DrawCalls++;
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotationAngle, const glm::vec4 color)
	{
		DBT_PROFILE_FUNCTION();

		// If we have drawn too many quads, we start a new batch
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();
		
		// Use the white texture
		const float texIndex = 0;

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), (rotationAngle), glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

		s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[0]);;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = glm::vec2(0.0f, 0.0f);
		s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[1]);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = glm::vec2(1.0f, 0.0f);
		s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[2]);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = glm::vec2(1.0f, 1.0f);
		s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[3]);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = glm::vec2(0.0f, 1.0f);
		s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotationAngle, const Ref<Texture>& texture, float tilingFactor)
	{
		DBT_PROFILE_FUNCTION();

		// If we have drawn too many quads, we start a new batch
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		//s_Data.TextureSlots[1] = texture;
		const glm::vec4 color = glm::vec4(1.0, 1.0, 1.0, 1.0);

		float textureIndex = 0.0f;
		for (int i = 0; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}
		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), (rotationAngle), glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		
		s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[0]);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = glm::vec2(0.0f, 0.0f);
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[1]);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = glm::vec2(1.0f, 0.0f);
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[2]);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = glm::vec2(1.0f, 1.0f);
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[3]);
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = glm::vec2(0.0f, 1.0f);
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Render2DStats));
	}
}