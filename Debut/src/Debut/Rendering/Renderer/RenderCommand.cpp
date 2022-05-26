#include "Debut/dbtpch.h"
#include "RenderCommand.h"
#include "RendererAPI.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Debut
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}