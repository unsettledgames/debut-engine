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
            Log.CoreError("[OpenGL Error] in %s: %d in function %s (ERROR CODE 0x%x)", file, line, function, error);
            std::cout << "[OpenGL Error] in " << file << ": " << line << " in " << function << " (" << error << ")" << std::endl;
            return false;
        }

        return true;
    }
}

