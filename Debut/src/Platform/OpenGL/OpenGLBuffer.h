#pragma once

#include "Debut/Renderer/Buffer.h"

namespace Debut
{
	////////////////////////////////////////////////////// VERTEX BUFFER /////////////////////////////////////////////////////
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(float* vertices, unsigned int size);
		virtual ~OpenGLVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetData(const void* data, uint32_t size) override;

		virtual inline void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		virtual inline BufferLayout& GetLayout() override { return m_Layout; }

		//static VertexBuffer* Create(float* vertices, unsigned int size);
	private:
		BufferLayout m_Layout;
		unsigned int m_RendererID;
	};

	////////////////////////////////////////////////////// INDEX BUFFER /////////////////////////////////////////////////////

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(int* indices, unsigned int size);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		
		virtual inline uint32_t GetCount() const override { return m_Count; }

		//static IndexBuffer* Create(int* vertices, unsigned int size);
	private:
		unsigned int m_RendererID;
		uint32_t m_Count;
	};
}

