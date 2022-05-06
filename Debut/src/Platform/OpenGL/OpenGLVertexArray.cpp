#include "Debut/dbtpch.h"
#include <glad/glad.h>
#include "OpenGLVertexArray.h"

namespace Debut
{

	static GLenum ShaderDataTypeToOpenGL(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:
		case ShaderDataType::Float2:
		case ShaderDataType::Float3:
		case ShaderDataType::Float4:
			return GL_FLOAT;

		case ShaderDataType::Int:
		case ShaderDataType::Int2:
		case ShaderDataType::Int3:
		case ShaderDataType::Int4:
			return GL_INT;

		case ShaderDataType::Mat3:
		case ShaderDataType::Mat4:
			return GL_FLOAT;

		case ShaderDataType::Bool:
			return GL_BOOL;
		}

		DBT_ASSERT(false, "Can't convert unknown ShaderDataType");
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		DBT_PROFILE_FUNCTION();
		glCreateVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		DBT_PROFILE_FUNCTION();
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& buffer)
	{
		glBindVertexArray(m_RendererID);
		buffer->Bind();

		uint32_t index = 0;
		for (const auto& element : buffer->GetLayout())
		{
			switch (element.Type)
			{
				case ShaderDataType::Float:
				case ShaderDataType::Float2:
				case ShaderDataType::Float3:
				case ShaderDataType::Float4:
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4:
				{
					glEnableVertexAttribArray(index);
					glVertexAttribPointer(index, element.GetComponentCount(), ShaderDataTypeToOpenGL(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE, buffer->GetLayout().GetStride(), (const void*)element.Offset);
					index++;
				}
				break;
				case ShaderDataType::Int:
				case ShaderDataType::Int3:
				case ShaderDataType::Int2:
				case ShaderDataType::Int4:
				case ShaderDataType::Bool:
				{
					glEnableVertexAttribArray(index);
					glVertexAttribIPointer(index, element.GetComponentCount(), ShaderDataTypeToOpenGL(element.Type),
						buffer->GetLayout().GetStride(), (const void*)element.Offset);
					index++;
				}
				break;

			}
			

			
		}
		m_VertexBuffers.push_back(buffer);
		
	}
	
	void OpenGLVertexArray::AddIndexBuffer(const Ref<IndexBuffer>& buffer)
	{
		glBindVertexArray(m_RendererID);

		buffer->Bind();
		m_IndexBuffer = buffer;
	}
}
