#include <Platform/OpenGL/OpenGLSkybox.h>
#include <Platform/OpenGL/OpenGLError.h>
#include <stb_image.h>

namespace Debut
{
	OpenGLSkybox::OpenGLSkybox(const std::string& front, const std::string& bottom, const std::string& left,
		const std::string& right, const std::string& up, const std::string& down)
	{
        std::vector<std::string> facesPaths = { front, bottom, left, right, up, down };
        std::vector<GLuint> faces = { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y };
        GLCall(glGenTextures(1, &m_RendererID));
        GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));
        stbi_set_flip_vertically_on_load(0);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char* data = stbi_load(facesPaths[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                if (nrChannels == 4)
                {
                    GLCall(glTexImage2D(faces[i], 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
                }
                else
                {
                    GLCall(glTexImage2D(faces[i], 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
                }
                stbi_image_free(data);
            }
            else
            {
                Log.CoreInfo("Cubemap texture failed to load at path: {0}", facesPaths[i]);
                stbi_image_free(data);
            }
        }

        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
	}

	OpenGLSkybox::~OpenGLSkybox()
	{
        Log.CoreInfo("Skybox destroyed");
        GLCall(glDeleteTextures(1, &m_RendererID));
	}

    void OpenGLSkybox::Bind()
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
        GLCall(glDepthMask(GL_FALSE));
    }

    void OpenGLSkybox::Unbind()
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        GLCall(glDepthMask(GL_TRUE));
    }
}