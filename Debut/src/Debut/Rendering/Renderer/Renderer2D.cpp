#include <Debut/dbtpch.h>

#include <Debut/Rendering/Structures/VertexArray.h>
#include <Debut/Rendering/Structures/Buffer.h>

#include <Debut/Rendering/Texture.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/Rendering/SubTexture2D.h>

#include <Debut/Rendering/Renderer/RenderCommand.h>
#include <Debut/Rendering/Shader.h>
#include <Debut/AssetManager/AssetManager.h>
#include <Debut/Rendering/Renderer/Renderer2D.h>
#include <Debut/Scene/Components.h>

#include <glm/gtc/matrix_transform.hpp>

namespace Debut
{
	Renderer2DStorage Renderer2D::s_Data;

	void Renderer2D::Init()
	{
		DBT_PROFILE_FUNCTION();

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
		s_Data.QuadVertexBuffer = VertexBuffer::Create((uint32_t)0, s_Data.MaxVertices * sizeof(QuadVertex));
		s_Data.QuadVertexArray = VertexArray::Create();
		BufferLayout squareLayout = {
			{ShaderDataType::Float3, "a_Position", false},
			{ShaderDataType::Float2, "a_UV", false},
			{ShaderDataType::Float4, "a_Color", false},
			{ShaderDataType::Float, "a_TexIndex", false},
			{ShaderDataType::Float, "a_TilingFactor", false},
			{ShaderDataType::Int, "a_EntityID", false}
		};
		s_Data.QuadVertexBuffer->SetLayout(squareLayout);

		// Setup vertex array
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);
		s_Data.QuadVertexArray->AddIndexBuffer(textIndBuffer);
		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		s_Data.LineVertexArray = VertexArray::Create();
		s_Data.LineVertexBuffer = VertexBuffer::Create((uint32_t)0, s_Data.MaxVertices * sizeof(LineVertex));
		BufferLayout lineLayout = {
			{ShaderDataType::Float3, "a_Position", false},
			{ShaderDataType::Float4, "a_Color", false}
		};
		s_Data.LineVertexBuffer->SetLayout(lineLayout);
		s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
		s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];

		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t data = 0xffffffff;
		s_Data.WhiteTexture->SetData(&data, sizeof(data));

		int samplers[s_Data.MaxTextureSlots];
		for (int i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		// Texture / shader setup
		s_Data.TextureShader = Shader::Create("assets/shaders/texture.glsl");
		s_Data.LineShader = Shader::Create("assets/shaders/line.glsl");
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

	void Renderer2D::BeginScene(Camera& camera, const glm::mat4 transform)
	{
		DBT_PROFILE_FUNCTION();

		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
		s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", viewProj);
		s_Data.TextureShader->SetMat4("u_ViewMatrix", glm::inverse(transform));
		s_Data.TextureShader->SetMat4("u_ProjectionMatrix", camera.GetProjection());

		s_Data.TextureShader->SetFloat("u_TilingFactor", 1.0f);
		s_Data.TextureShader->Unbind();

		s_Data.LineShader->Bind();
		s_Data.LineShader->SetMat4("u_ViewProjection", viewProj);
		s_Data.LineShader->SetMat4("u_ViewMatrix", glm::inverse(transform));
		s_Data.LineShader->SetMat4("u_ProjectionMatrix", camera.GetProjection());
		s_Data.LineShader->Unbind();

		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		DBT_PROFILE_FUNCTION();
		if (s_Data.QuadIndexCount)
		{
			uint64_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

			s_Data.TextureShader->Bind();
			Flush();
			s_Data.TextureShader->Unbind();
		}

		if (s_Data.LineVertexCount)
		{
			uint64_t dataSize = (uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase;
			s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);

			s_Data.LineShader->Bind();			
			RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
			s_Data.LineShader->Unbind();
		}
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
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		// Draw call
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		s_Data.Stats.DrawCalls++;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color)
	{
		s_Data.LineVertexBufferPtr->Position = p0;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexBufferPtr->Position = p1;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexCount += 2;
	}

	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec2& size, const glm::vec2& offset,const glm::vec4& color)
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
		glm::vec2 texCoords[] = { glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1) };

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[i]);
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = 0;
			s_Data.QuadVertexBufferPtr->TilingFactor = 1;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4 color)
	{
		DBT_PROFILE_FUNCTION();

		// If we have drawn too many quads, we start a new batch
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		// Use the white texture
		const float texIndex = 0;
		glm::vec2 texCoords[] = { glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1) };

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[i]);
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = 0;
			s_Data.QuadVertexBufferPtr->TilingFactor = 1;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotationAngle, const Ref<Texture>& texture, float tilingFactor)
	{
		DBT_PROFILE_FUNCTION();

		// If we have drawn too many quads, we start a new batch
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
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

		const glm::vec4 color = glm::vec4(1.0, 1.0, 1.0, 1.0);
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), (rotationAngle), glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		glm::vec2 texCoords[] = { glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1) };

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[i]);
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotationAngle, const Ref<SubTexture2D>& texture, float tilingFactor)
	{
		DBT_PROFILE_FUNCTION();

		// If we have drawn too many quads, we start a new batch
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *(texture->GetTexture().get()))
			{
				textureIndex = (float)i;
				break;
			}
		}
		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture->GetTexture();
			s_Data.TextureSlotIndex++;
		}

		const glm::vec4 color = glm::vec4(1.0, 1.0, 1.0, 1.0);
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), (rotationAngle), glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		glm::vec2 texCoords[] = { glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1) };

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[i]);
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = texture->GetTexCoords()[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& src, int entityID)
	{
		DBT_PROFILE_FUNCTION();

		// If we have drawn too many quads, we start a new batch
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		Ref<Texture2D> texture = AssetManager::Request<Texture2D>(src.Texture);

		float textureIndex = 0.0f;
		if (src.Texture)
		{
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			{
				if (*s_Data.TextureSlots[i].get() == *(texture.get()))
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
		}

		glm::vec2 texCoords[] = { glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1) };

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = glm::vec3(transform * s_Data.QuadVertexPositions[i]);
			s_Data.QuadVertexBufferPtr->Color = src.Color;
			s_Data.QuadVertexBufferPtr->TexCoord = texCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = src.TilingFactor;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Render2DStats));
	}

	void Renderer2D::StartBatch()
	{
		s_Data.TextureSlotIndex = 1;
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.LineVertexCount = 0;
		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;
	}
}