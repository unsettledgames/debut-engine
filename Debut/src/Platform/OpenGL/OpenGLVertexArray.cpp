#include "Debut/dbtpch.h"
#include <glad/glad.h>
#include "OpenGLVertexArray.h"
#include <Debut/Rendering/Structures/Buffer.h>
#include <Debut/Rendering/Shader.h>
#include <Platform/OpenGL/OpenGLError.h>

namespace Debut
{

	static GLenum ShaderAttribTypeToOpenGL(ShaderDataType type)
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

		DBT_ASSERT(false, "Can't convert unknown ShaderAttribType");
		return GL_NONE;
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
		GLCall(glBindVertexArray(m_RendererID));
		if (m_IndexBuffer != nullptr)
			m_IndexBuffer->Bind();
	}

	void OpenGLVertexArray::Unbind() const
	{
		if (m_IndexBuffer != nullptr)
			m_IndexBuffer->Unbind();
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& buffer)
	{
		GLCall(glBindVertexArray(m_RendererID));
		buffer->Bind();

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
					GLCall(glEnableVertexAttribArray(m_AttributeIndex));
					GLCall(glVertexAttribPointer(m_AttributeIndex, element.GetComponentCount(), ShaderAttribTypeToOpenGL(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE, buffer->GetLayout().GetStride(), (const void*)element.Offset));
					m_AttributeIndex++;
				}
				break;
				case ShaderDataType::Int:
				case ShaderDataType::Int3:
				case ShaderDataType::Int2:
				case ShaderDataType::Int4:
				case ShaderDataType::Bool:
				{
					glEnableVertexAttribArray(m_AttributeIndex);
					glVertexAttribIPointer(m_AttributeIndex, element.GetComponentCount(), ShaderAttribTypeToOpenGL(element.Type),
						buffer->GetLayout().GetStride(), (const void*)element.Offset);
					m_AttributeIndex++;
				}
				break;

			}
		}
		m_VertexBuffers.push_back(buffer);
		buffer->Unbind();
		glBindVertexArray(0);
	}
	
	void OpenGLVertexArray::AddIndexBuffer(const Ref<IndexBuffer>& buffer)
	{
		glBindVertexArray(m_RendererID);
		m_IndexBuffer = buffer;
		glBindVertexArray(0);
	}
}
