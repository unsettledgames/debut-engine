#include "Debut/dbtpch.h"
#include <glad/glad.h>
#include "OpenGLBuffer.h"

namespace Debut
{
	////////////////////////////////////////////////////// VERTEX BUFFER /////////////////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, unsigned int count)
	{
		DBT_PROFILE_FUNCTION();
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * count, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
	{
		DBT_PROFILE_FUNCTION();
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	void OpenGLVertexBuffer::PushData(const void* data, uint32_t size)
	{
		uint32_t newDataSize = m_DataSize;
		// Increase buffer size if necessary
		while (m_DataIndex + size >= m_DataSize)
			newDataSize *= 2;

		if (newDataSize != m_DataSize)
		{
			char* newData = new char[newDataSize];
			memcpy(newData, m_Data, m_DataSize);

			delete m_Data;
			m_Data = newData;
			m_DataSize = newDataSize;
		}

		memcpy((char*)m_Data + m_DataSize, data, size);
		m_DataIndex += size;
	}

	void OpenGLVertexBuffer::SubmitData()
	{
		SetData(m_Data, m_DataIndex);
		m_DataIndex = 0;
		m_DataSize = 4096;
	}

	void OpenGLVertexBuffer::Bind() const
	{
		DBT_PROFILE_FUNCTION();
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	////////////////////////////////////////////////////// INDEX BUFFER /////////////////////////////////////////////////////


	OpenGLIndexBuffer::OpenGLIndexBuffer(int* indices, unsigned int count)
	{
		DBT_PROFILE_FUNCTION();
		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_UNSIGNED_INT) * count, indices, GL_STATIC_DRAW);

		m_Count = count;
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t count)
	{
		DBT_PROFILE_FUNCTION();
		glCreateBuffers(1, &m_RendererID);

		m_Count = 0;
	}

	void OpenGLIndexBuffer::Bind() const
	{
		DBT_PROFILE_FUNCTION();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::SetData(const void* data, uint32_t count)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_UNSIGNED_INT) * count, data, GL_STATIC_DRAW);

		m_Count = count;
	}
}
