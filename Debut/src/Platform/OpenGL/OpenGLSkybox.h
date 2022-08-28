#pragma once

#include <glad/glad.h>
#include <Debut/Rendering/Resources/Skybox.h>

namespace Debut
{
	class OpenGLSkybox : public Skybox
	{
	public:
		OpenGLSkybox() {};
		OpenGLSkybox(const OpenGLSkybox&) {};
		OpenGLSkybox(const std::string& front, const std::string& bottom, const std::string& left,
			const std::string& right, const std::string& up, const std::string& down);
		~OpenGLSkybox();

		virtual void Bind() override;
		virtual void Unbind() override;

		inline uint32_t GetRendererID() const override { return m_RendererID; }

	private:
		GLuint m_RendererID = 0;
	};
}