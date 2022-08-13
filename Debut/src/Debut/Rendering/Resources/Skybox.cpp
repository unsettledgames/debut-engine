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
			std::vector<float> cubeVertices = {-4, -4,  4,	4, -4, 4,	-4,  4,  4,	4,  4,  4,	-4, -4, -4,	 4, -4, -4,	-4,  4, -4, 4,  4, -4 };
			std::vector<int> cubeIndices = { 2, 6, 7, 2, 3, 7, 0, 4, 5, 0, 1, 5, 0, 2, 6, 0, 4, 6, 1, 3, 7, 1, 5, 7, 0, 2, 3,
				0, 1, 3, 4, 6, 7, 4, 5, 7 };
            ret->m_Mesh.SetPositions(cubeVertices);
			ret->m_Mesh.SetIndices(cubeIndices);
			ret->m_Material.SetShader(shader);

			return ret;
		}
	}
}