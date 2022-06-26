#pragma once

#include "Debut/Rendering/Structures/Buffer.h"

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
		virtual void PushData(const void* data, uint32_t size) override;
		virtual void SubmitData() override;

		virtual inline void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		virtual inline BufferLayout& GetLayout() override { return m_Layout; }
	private:
		BufferLayout m_Layout;
		unsigned int m_RendererID;

		void* m_Data;
		uint32_t m_DataIndex = 0;
		uint32_t m_DataSize = 4096;
	};

	////////////////////////////////////////////////////// INDEX BUFFER /////////////////////////////////////////////////////

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(int* indices, unsigned int size);
		OpenGLIndexBuffer(uint32_t count);
		virtual ~OpenGLIndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		
		virtual inline uint32_t GetCount() const override { return m_Count; }

		virtual void SetData(const void* data, uint32_t count) override;
	private:
		unsigned int m_RendererID;
		uint32_t m_Count;
	};
}

