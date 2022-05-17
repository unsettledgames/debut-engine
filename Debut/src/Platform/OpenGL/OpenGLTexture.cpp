#include "Debut/dbtpch.h"
#include "OpenGLTexture.h"
#include <stb_image.h>
#include <Debut/Core/Log.h>
#include "OpenGLError.h"

namespace Debut
{
	static GLenum DbtToGLParameter(Texture2DParameter parameter)
	{
		switch (parameter)
		{
		case Texture2DParameter::FILTERING_LINEAR: return GL_LINEAR;
		case Texture2DParameter::FILTERING_POINT: return GL_NEAREST;
		case Texture2DParameter::WRAP_CLAMP: return GL_CLAMP_TO_EDGE;
		case Texture2DParameter::WRAP_REPEAT: return GL_REPEAT;
		default: DBT_ASSERT(false, "Couldn't convert texture parameter from Dbt to GL"); return GL_NONE;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, Texture2DConfig config) : m_Path(path)
	{
		DBT_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_uc* data = nullptr;
		stbi_set_flip_vertically_on_load(1);

		{
			DBT_PROFILE_SCOPE("OpenGLTexture2D::OpenGLTexture2D(const std::string&)");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}

		DBT_CORE_ASSERT(data, "Failed to load png image");

		m_Width = width;
		m_Height = height;
		m_MinFiltering = config.Filtering;
		m_MagFiltering = config.Filtering;
		m_WrapMode = config.WrapMode;

		if (channels == 4)
		{
			m_InternalFormat = GL_RGBA8;
			m_Format = GL_RGBA;
		}
		else if (channels == 3)
		{
			m_InternalFormat = GL_RGB8;
			m_Format = GL_RGB;
		}

		DBT_CORE_ASSERT(m_Format != 0 && m_InternalFormat != 0, "Png texture format not supported");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, DbtToGLParameter(config.Filtering)));
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, DbtToGLParameter(config.Filtering)));
		
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, DbtToGLParameter(config.WrapMode)));
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, DbtToGLParameter(config.WrapMode)));

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_Format, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	void OpenGLTexture2D::Reload()
	{
		int width, height, channels;
		stbi_uc* data = nullptr;
		stbi_set_flip_vertically_on_load(1);

		{
			DBT_PROFILE_SCOPE("OpenGLTexture2D::OpenGLTexture2D(const std::string&)");
			data = stbi_load(m_Path.c_str(), &width, &height, &channels, 0);
		}

		DBT_CORE_ASSERT(data, "Failed to load png image");

		if (channels == 4)
		{
			m_InternalFormat = GL_RGBA8;
			m_Format = GL_RGBA;
		}
		else if (channels == 3)
		{
			m_InternalFormat = GL_RGB8;
			m_Format = GL_RGB;
		}

		DBT_CORE_ASSERT(m_Format != 0 && m_InternalFormat != 0, "Png texture format not supported");
		if (m_RendererID != 0)
			glDeleteTextures(1, &m_RendererID);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, DbtToGLParameter(m_MinFiltering)));
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, DbtToGLParameter(m_MagFiltering)));

		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, DbtToGLParameter(m_WrapMode)));
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, DbtToGLParameter(m_WrapMode)));

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_Format, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
	{
		DBT_PROFILE_FUNCTION();
		m_InternalFormat = GL_RGBA8, m_Format = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		DBT_PROFILE_FUNCTION();
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_Format, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		DBT_PROFILE_FUNCTION();
		glBindTextureUnit(slot, m_RendererID);
	}

}