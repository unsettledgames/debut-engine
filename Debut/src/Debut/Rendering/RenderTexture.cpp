#include <Debut/dbtpch.h>
#include <Debut/Rendering/RenderTexture.h>
#include <Debut/Rendering/Structures/FrameBuffer.h>
#include <Debut/Rendering/Renderer/RendererAPI.h>

#include <Platform/OpenGL/OpenGLRenderTexture.h>>

namespace Debut
{
	Ref<RenderTexture> RenderTexture::Create(float width, float height, Ref<FrameBuffer> buffer)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLRenderTexture>(width, height, buffer);
		default:
			return nullptr;
		}
	}
}