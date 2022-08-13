#pragma once

#include <glad/glad.h>
#include <Debut/Rendering/Resources/Skybox.h>

namespace Debut
{
	class OpenGLSkybox : public Skybox
	{
	public:
		OpenGLSkybox(const std::string& front, const std::string& bottom, const std::string& left,
			const std::string& right, const std::string& up, const std::string& down);
		~OpenGLSkybox();

		GLuint OpenGLSkybox::GetRendererID() override { return m_RendererID; }

		virtual void Bind() override;
		virtual void Unbind() override;

	private:
		GLuint m_RendererID;
	};
}