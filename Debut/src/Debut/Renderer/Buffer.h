#pragma once

#include "Debut/Core/Core.h"
#include "Debut/Core/Log.h"

namespace Debut
{
	enum class ShaderDataType : uint8_t
	{
		None = 0, 
		Float, Float2, Float3, Float4, 
		Int, Int2, Int3, Int4, 
		Bool,
		Mat3, Mat4, Struct
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::None: return -1;

			case ShaderDataType::Float: return 4;
			case ShaderDataType::Float2: return 4 * 2;
			case ShaderDataType::Float3: return 4 * 3;
			case ShaderDataType::Float4: return 4 * 4;

			case ShaderDataType::Int: return 4;
			case ShaderDataType::Int2: return 4 * 2;
			case ShaderDataType::Int3: return 4 * 3;
			case ShaderDataType::Int4: return 4 * 4;

			case ShaderDataType::Mat3: return 4 * 3 * 3;
			case ShaderDataType::Mat4: return 4 * 4 * 4;

			case ShaderDataType::Bool: return 1;
		}

		DBT_ASSERT(false, "Unknown ShaderDataType");

		return 0;
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;

		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement() {}

		BufferElement(ShaderDataType type, const std::string& name, bool normalize) :
			Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalize) {}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
			case ShaderDataType::Float: return 1;
			case ShaderDataType::Float2: return 2;
			case ShaderDataType::Float3: return 3;
			case ShaderDataType::Float4: return 4;

			case ShaderDataType::Int: return 1;
			case ShaderDataType::Int2: return 2;
			case ShaderDataType::Int3: return 3;
			case ShaderDataType::Int4: return 4;

			case ShaderDataType::Mat3: return 3 * 3;
			case ShaderDataType::Mat4: return 4 * 4;

			case ShaderDataType::Bool: return 1;
			}

			DBT_ASSERT(false, "Unknown ShaderDataType");
			return 0;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(const std::initializer_list<BufferElement>& elements) : m_Elements(elements) 
		{
			CalculateOffsetStride();
		}

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }

		inline const std::vector<BufferElement> GetElements() { return m_Elements; }
		inline uint32_t GetStride() const { return m_Stride; }

	private:
		void CalculateOffsetStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;

			for (auto& element : m_Elements)
			{
				element.Offset = offset;

				m_Stride += element.Size;
				offset += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride;
		
	};


	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		
		virtual inline void SetLayout(const BufferLayout& layout) = 0;
		virtual inline BufferLayout& GetLayout() = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(float* vertices, unsigned int count);
	private:
	};


	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		
		virtual uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(int* indices, unsigned int count);
	private:
	};
}
