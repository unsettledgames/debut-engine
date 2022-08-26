#pragma once
#include <Debut/dbtpch.h>

#include <Debut/Core/Log.h>
#include <Debut/Core/Core.h>
#include <Debut/Rendering/Renderer/Renderer.h>
#include <Debut/Rendering/Renderer/RendererAPI.h>
#include <Platform/OpenGL/OpenGLBuffer.h>
#include <Debut/Rendering/Structures/Buffer.h>

namespace Debut
{
	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, unsigned int count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(vertices, count);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, uint32_t bufferSize)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>((uint32_t)size, bufferSize);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(int* indices, unsigned int count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(indices, count);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>();
		}

		DBT_ASSERT(false, "Unsupported renderer API");
		return nullptr;
	}
}