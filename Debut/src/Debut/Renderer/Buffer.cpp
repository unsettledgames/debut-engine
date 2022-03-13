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
	VertexBuffer* VertexBuffer::Create(float* vertices, unsigned int count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexBuffer(vertices, count);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
	}

	IndexBuffer* IndexBuffer::Create(int* indices, unsigned int count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLIndexBuffer(indices, count);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
	}
}