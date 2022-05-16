#pragma once

#include "Debut/Core/Core.h"

namespace Debut
{
	enum class Texture2DParameter
	{
		FILTERING_POINT = 0, FILTERING_LINEAR,
		WRAP_REPEAT, WRAP_CLAMP, NONE
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual std::string GetPath() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual bool operator==(const Texture& other) const = 0;
		
	};

	class Texture2D : public Texture
	{
	protected:
		// Texture parameters
		Texture2DParameter m_MinFiltering = Texture2DParameter::FILTERING_LINEAR;
		Texture2DParameter m_MagFiltering = Texture2DParameter::FILTERING_LINEAR;
		Texture2DParameter m_WrapMode = Texture2DParameter::WRAP_CLAMP;

	public:
		static Ref<Texture2D> Create(const std::string& path);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);

		Texture2DParameter GetMinFiltering() { return m_MinFiltering; }
		Texture2DParameter GetMagFiltering() { return m_MagFiltering; }
		Texture2DParameter GetWrapMode() { return m_WrapMode; }

		void SetMinFiltering(Texture2DParameter param) { m_MinFiltering = param; }
		void SetMagFiltering(Texture2DParameter param) { m_MagFiltering = param; }
		void SetWrapMode(Texture2DParameter param) { m_WrapMode = param; }

		float GetAspectRatio() { return (float)GetWidth() / GetHeight(); }
	};
}
