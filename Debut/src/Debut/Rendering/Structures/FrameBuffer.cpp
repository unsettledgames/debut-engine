#include "Debut/dbtpch.h"
#include <Debut/Rendering/Structures/FrameBuffer.h>
#include <Debut/Rendering/Renderer/RendererAPI.h>
#include <Platform/OpenGL/OpenGLFrameBuffer.h>

namespace Debut
{
	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecifications& specs)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLFrameBuffer>(specs);
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			break;
		}

		return nullptr;
	}
}