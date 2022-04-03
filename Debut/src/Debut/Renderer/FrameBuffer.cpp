#include "Debut/dbtpch.h"
#include "FrameBuffer.h"
#include "RendererAPI.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"

namespace Debut
{
	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecs& specs)
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