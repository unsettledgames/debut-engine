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
            Log.CoreError("[OpenGL Error] in {0}: {1} in function {2} (ERROR CODE 0x{3:x})", file, line, function, error);
            return false;
        }

        return true;
    }
}

