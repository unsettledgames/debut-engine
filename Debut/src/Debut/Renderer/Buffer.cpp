#pragma once
#include <Debut/dbtpch.h>

#include "Debut/Log.h"
#include "Debut/Core.h"
#include "Renderer.h"
#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Buffer.h"

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
			return std::make_shared<OpenGLVertexBuffer>(vertices, count);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
	}

	Ref<IndexBuffer> IndexBuffer::Create(int* indices, unsigned int count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLIndexBuffer>(indices, count);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
	}
}