#include "Debut/dbtpch.h"

#include <Debut/Rendering/Renderer/Renderer.h>
#include <Debut/Rendering/Renderer/RendererAPI.h>
#include <Debut/Rendering/Structures/Buffer.h>
#include <Debut/Rendering/Structures/VertexArray.h>
#include <Platform/OpenGL/OpenGLVertexArray.h>

namespace Debut
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLVertexArray>();
		}

		DBT_ASSERT(false, "Unsupported renderer API");
		return nullptr;
	}	
}