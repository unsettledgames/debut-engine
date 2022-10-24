#include "Debut/dbtpch.h"
#include "Renderer.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "Renderer3D.h"
#include "RendererDebug.h"

namespace Debut
{
	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData();

	void Renderer::Init()
	{
		RenderCommand::Init();
		RendererDebug::Init();
		Renderer2D::Init();
		Renderer3D::Init();
	}

	void Renderer::OnWindowResized(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::ToggleTextures(bool val)
	{
		Renderer2D::ToggleTextures(val);
		Renderer3D::ToggleWireframe(val);
	}

	void Renderer::ToggleWireframe(bool val)
	{
		Renderer2D::ToggleWireframe(val);
		Renderer3D::ToggleWireframe(val);
	}
}