#pragma once

#include <glad/glad.h>
#include "Debut/Renderer/Texture.h"

namespace Debut
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path, Texture2DConfig parameters = 
			{Texture2DParameter::FILTERING_LINEAR, Texture2DParameter::WRAP_CLAMP});
		OpenGLTexture2D(uint32_t width, uint32_t height);
		virtual ~OpenGLTexture2D();

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }
		uint32_t GetRendererID() const override { return m_RendererID; }
		std::string GetPath() const override { return m_Path; }

		virtual void SetData(void* data, uint32_t size) override;
		virtual void Reload() override;

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override{ return ((OpenGLTexture2D&)other).GetRendererID() == m_RendererID; }

		

	private:
		std::string m_Path;
		uint32_t m_RendererID;

		uint32_t m_Width;
		uint32_t m_Height;
		GLenum m_InternalFormat;
		GLenum m_Format;
	};
}

