#include <Debut/Core/Core.h>
#include <Debut/Rendering/Resources/Skybox.h>
#include <Platform/OpenGL/OpenGLSkybox.h>
#include "Debut/dbtpch.h"
#include "Debut/Rendering/Renderer/Renderer.h"

namespace Debut
{
	Ref<Skybox> Skybox::Create(const std::string& front, const std::string& bottom, const std::string& left, 
		const std::string& right, const std::string& up, const std::string& down, Ref<Shader> shader)
	{
		Ref<Skybox> ret = nullptr;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:
			ret = CreateRef<OpenGLSkybox>(front, bottom, left, right, up, down);
		}

		if (ret != nullptr)
		{
			std::vector<float> cubeVertices = {
				-1.0f,  1.0f, -1.0f,    -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f,     1.0f, -1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,    -1.0f,  1.0f, -1.0f,    -1.0f, -1.0f,  1.0f,    -1.0f, -1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,    -1.0f,  1.0f, -1.0f,    -1.0f,  1.0f,  1.0f,    -1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f, -1.0f,    1.0f, -1.0f,  1.0f,     1.0f,  1.0f,  1.0f,     1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f, -1.0f,    1.0f, -1.0f, -1.0f,     -1.0f, -1.0f,  1.0f,    -1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f,     1.0f, -1.0f,  1.0f,     -1.0f, -1.0f,  1.0f,
				-1.0f,  1.0f, -1.0f,    1.0f,  1.0f, -1.0f,     1.0f,  1.0f,  1.0f,      1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,    -1.0f,  1.0f, -1.0f,    -1.0f, -1.0f, -1.0f,    -1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f, -1.0f,	 1.0f, -1.0f, -1.0f,	-1.0f, -1.0f,  1.0f,	1.0f, -1.0f,  1.0f
			};
            ret->m_Mesh.SetPositions(cubeVertices);
			ret->m_Material.SetShader(shader);

			return ret;
		}
	}
}