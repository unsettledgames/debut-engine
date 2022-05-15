#include "Debut/dbtpch.h"

#include "Debut/Renderer/Renderer.h"
#include "Debut/Renderer/RendererAPI.h"
#include "Debut/Renderer/VertexArray.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

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
			return std::make_shared<OpenGLVertexArray>();
		}

		DBT_ASSERT(false, "Unsupported renderer API");
		return nullptr;
	}	
}