#pragma once

#include <Debut/dbtpch.h>
#include <Debut/Log.h>
#include <Debut/Renderer/Renderer.h>
#include <Debut/Renderer/Shader.h>
#include <Platform/OpenGL/OpenGLShader.h>

namespace Debut
{
	Ref<Shader> Shader::Create(const std::string& vertSrc, const std::string& fragSrc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			DBT_ASSERT(false, "The renderer doesn't have an API set.");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLShader>(vertSrc, fragSrc);
		}

		DBT_ASSERT(false, "Unsupported renderer API");
	}
}
