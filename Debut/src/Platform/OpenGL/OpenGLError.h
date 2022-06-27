#pragma once

#include <glad/glad.h>
#include <Debut/Core/Log.h>

#ifdef DBT_DEBUG

#define GLCall(x)   GLClearError(); \
                    x;              \
                    GLLogCall(#x, __FILE__, __LINE__);

#else

#define GLCall(x) x;

#endif

namespace Debut
{
    inline void GLClearError() { while (glGetError()); }


    inline bool GLLogCall(const char* function, const char* file, int line)
    {
        while (GLenum error = glGetError())
        {
            std::string err;
            switch (error)
            {
            case GL_INVALID_ENUM:
                err = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_OPERATION:
                err = "GL_INVALID_OPERATION";
                break;
            case GL_INVALID_VALUE:
                err = "GL_INVALID_VALUE";
                break;
            default:
                err = "Un altro infame";
                break;
            }
            Log.CoreError("[OpenGL Error] in {0}: {1} in function {2} (ERROR CODE 0x{3:x}, {4}", file, line, function, error, err);
            return false;
        }

        return true;
    }
}

